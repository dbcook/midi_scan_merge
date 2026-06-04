#include <Arduino.h>
#include "glob_gen.h"
#include "platform_serial.h"

#if 0
void UartToMidiInterfaceMap::initMidiMapList() {
    for (size_t i = 0; i < numHardwareUartPorts; i++) {
        UartToMidiInterfaceMap newItem(&Serial1, pMidi0);
        midiMapList.add(newItem);
    }
}
#endif