#pragma once

#include <MIDI.h>


namespace audio {



using NoteNumber = byte;
using CCNumber = byte;



struct Synth {
    /// @brief Play a new note.
    /// @param note the MIDI number of the note.
    /// @param velocity the normalized (0-1) velocity/volume for the note
    void noteOn(NoteNumber note, float velocity);

    /// @brief End playing the given note.
    /// @param note the MIDI number of the note.
    /// @param velocity the normalized (0-1) velocity/volume for the note as it ends.
    void noteOff(NoteNumber note, float velocity);

    /// @brief Process the given control change if relevant.
    /// @param cc the number (0-127) of the control.
    /// @param value the new value for the control to adopt
    void controlChange(CCNumber cc, byte value);

    virtual ~Synth() {}
};

}