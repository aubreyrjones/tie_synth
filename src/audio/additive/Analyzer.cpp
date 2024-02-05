#include "Analyzer.hpp"

#include <SD.h>

namespace audio {

static boolean sdInit = false;
static boolean sdAvailable = false;

WavReader::~WavReader() {
    file.close();
}

void initialize_sd() {
    if (sdInit) return;

    if (!SD.begin(BUILTIN_SDCARD)) {
        sdAvailable = false;
        sdInit = true;
        Serial.println("Cannot initialize SD card.");

        return;
    }

    sdInit = true;
    sdAvailable = true;
}

bool sd_ready() {
    return sdInit && sdAvailable;
}

WavReader::WavReader(const char* path) : file() {
    initialize_sd();

    if (!sd_ready()) {
        Serial.println("No SD card inserted.");
        errorState = Error::NO_SD;
        return;
    }

    if (!SD.exists(path)) {
        Serial.print(path);
        Serial.println(". No such file.");
        errorState = Error::MISSING;
        return;
    }
    
    file = SD.open(path);
    if (!file.size()) {
        Serial.println("File cannot be opened.");
        errorState = Error::CORRUPT;
        return;
    }

    uint32_t longRegister;
    uint16_t shortRegister;

    auto read_check = [this, &longRegister](uint32_t magic) -> bool {
        file.readBytes((char*) &longRegister, 4);
        if (longRegister != magic) {
            Serial.print(longRegister);
            return false;
        }
        return true;
    };

    auto read_long = [&longRegister, this]() -> uint32_t& {
        file.readBytes((char*) &longRegister, sizeof(uint32_t));
        return longRegister;
    };

    auto read_short = [&shortRegister, this]() -> uint16_t& {
        file.readBytes((char*) &shortRegister, sizeof(uint16_t));
        return shortRegister;
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar" // doesn't actually help, but it's supposed to.
    if (!read_check('FFIR')) {
        errorState = Error::UNSUPPORTED;
        return;
    }

    file.seek(4, SEEK_CUR); //skip the file length

    if (!read_check('EVAW')) {
        errorState = Error::UNSUPPORTED;
        return;
    }

    if (!read_check(' tmf')) {
        errorState = Error::UNSUPPORTED;
        return;
    }

    if (read_long() != 16) {
        Serial.println(longRegister);
        errorState = Error::UNSUPPORTED;
        return;
    }

    auto type = read_short();
    if (type != 1) { // PCM
        Serial.println(type);
        errorState = Error::UNSUPPORTED;
        return;
    }

    nChannels = read_short();
    
    auto sampleRate = read_long();

    if (sampleRate != 44100) {
        Serial.println(sampleRate);
        errorState = Error::UNSUPPORTED;
        return;
    }

    file.seek(6, SEEK_CUR); // skip byterate and block alignment
    auto bps = read_short();
    if (bps != 16) {
        Serial.println(bps);
        errorState = Error::UNSUPPORTED;
        return;
    }

    if (!read_check('atad')) {
        errorState = Error::CORRUPT;
        return;
    }

    auto chunkSize = read_long();
    nSamples = chunkSize / (nChannels * sizeof(int16_t)); // we only support 16-bit samples, so this is "fine"
#pragma GCC diagnostic pop
}

void WavReader::readSamples(sample **buffers, uint32_t nSamples) {
    int16_t readbuf[64];
    int outputIndex = 0;

    while (nSamples) {
        file.read((char*) readbuf, nChannels * sizeof(int16_t));

        for (int i = 0; i < nChannels; i++) {
            if (buffers[i])
                buffers[i][outputIndex] = readbuf[i];
        }

        nSamples--;
        outputIndex++;
    }
}


bool AudioAnalyzer::loadFromSD(const char* path) {
    reader = std::make_unique<WavReader>(path);

    return true;
}


}