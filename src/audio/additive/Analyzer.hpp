#pragma once

#include <cstdint>
#include <string>
#include <FS.h>
#include <memory>

namespace audio {

using sample = int16_t;

/**
 * Basic wave file reader for 44100/16 audio.
*/
class WavReader {
public:
    enum class Error {
        OK,
        MISSING,
        UNSUPPORTED,
        CORRUPT,
        NO_SD
    };

protected:
    File file;

    int nSamples = -1;
    int nChannels = -1;
    int readIndex = -1;
    Error errorState = Error::OK;

public:
    WavReader(const char* path);
    virtual ~WavReader();

    /// @brief Get the status of the WavReader.
    Error status();

    /**
     * Get the number of samples in the wav file.
    */ 
    int length() const { return nSamples; };

    /**
     * Read nSamples into an array of buffers.
    */
    void readSamples(sample **buffers, uint32_t nSamples);
};


class AudioAnalyzer {
private:
    std::unique_ptr<WavReader> reader = nullptr;

public:
    bool loadFromSD(const char* path);

    WavReader* getReader() { return reader.get(); }
};

}