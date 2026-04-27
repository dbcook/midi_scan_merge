# MIDI Reference Info

## Note Numbering and Octave Conventions

MIDI note numbers lie in the interval [0, 127].

**By industry-wide convention, middle C (the first C below A440) is always MIDI note 60.** This means that MIDI note 0 is always a C, and all C
note numbers are multiples of 12.

**Octave numbering (e.g. C2, A6, etc.) is a complete free-for-all and there is no consistent industry standard.** There are two competing conventions:

| Convention | MIDI 0       | MIDI 127
| ----       | ----         | ----
| MidC = C4  | C(-1)        | G9
| MidC = C3  | C(-2)        | G8


## Usual MIDI Compass of Various Controllers

Note numbers on large standard pedalboards (27, 30 or 32 keys) start at MIDI note 36.

| Controller            | Base MIDI note | Top MIDI note
| ----                  | ----           | ----
| 49-note keyboard      | TBD            | TBD + 48
| 61-note keyboard      | 36             | 96
| 73-note keyboard      | 36             | 108
| 88-note keyboard      | 21             | 108
| 27-note pedalboard    | 36             | 62
| 30-note pedalboard    | 36             | 65
| 32-note pedalboard    | 36             | 67

## Unresolved

Do note numbers shift when transposing functions are active?


