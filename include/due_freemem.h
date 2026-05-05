#pragma once
#include <Arduino.h>



#if defined(ARDUINO_SAM_DUE)
#include <malloc.h>

extern char _end;
extern "C" char *sbrk(int i);

uint32_t freeMemory() {
  struct mallinfo mi = mallinfo();
  char *heapend = sbrk(0);
  register char *stack_ptr asm ("sp");

  return (uint32_t)(stack_ptr - heapend + mi.fordblks);
}

void display_freeram() {
  struct mallinfo mi = mallinfo();
  char *heapend = sbrk(0);
  register char *stack_ptr asm ("sp");

  Serial.print(F("Free RAM: "));
  Serial.println(stack_ptr - heapend + mi.fordblks);
}
#endif
