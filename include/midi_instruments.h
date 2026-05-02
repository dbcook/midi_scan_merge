#pragma once

// Define note number vs octave convention used locally.  This is chaotic across the industry,
// so it's best to avoid using octave-numbered constants where possible
#define MIDI_NOTENUM_MIDDLE_C   60                          // this is not in dispute
#define MIDI_NOTENUM_C4         MIDI_NOTENUM_MIDDLE_C       // this is local convention

#define MIDI_NOTENUM_A0     21         // low note on 88-key keyboard, MidC == C4 convention
#define MIDI_NOTENUM_C2     36         // low note on 61 and 73 key keyboards, MidC == C4 convention

// Instrument-related MIDI note number constants follow.  Ultra short pedalboards not considered.

// Common compasses of pedalboards
#define PEDAL27_MAX_NOTES   27
#define PEDAL30_MAX_NOTES   30
#define PEDAL32_MAX_NOTES   32

// All normal compass pedalboards start at 36 (2 octaves below middle C, 16' pipe)
#define PEDAL_LOW_NOTENUM 36

#define KEYBOARD61_LOW_NOTENUM 36
#define KEYBOARD73_LOW_NOTENUM 36
#define KEYBOARD88_LOW_NOTENUM 21          // hard constant so unambiguous

// Common compasses of MIDI enabled keyboards (49 key or better)
#define KEYBOARD49_MAX_NOTES 49
#define KEYBOARD61_MAX_NOTES 61
#define KEYBOARD73_MAX_NOTES 73
#define KEYBOARD88_MAX_NOTES 88
