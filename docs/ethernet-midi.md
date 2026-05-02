# Ethernet MIDI aka Apple Midi aka RTP-MIDI Support

This package uses the [Apple Midi library](https://github.com/lathoub/Arduino-AppleMIDI-Library)
which works in conjunction with the sstaub/Ethernet3 library and the 47Effects MIDI
library.

It will interoperate with the built-in Apple Midi on MacOS and with Tobias Erichsen's rtpMIDI
on Windows.

The Ethernet3 library is used instead of the default Arduino Ethernet library because it allows setting
MAX_SOCKETS down to 4, which in turn allows the WizNet 5500 to use larger buffers internally.
This helps prevent buffer overflows.
```
Ethernet.begin(4);      // 4 sockets, 4KB buffers each in wiz 5500
```
