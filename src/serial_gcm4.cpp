#include <Arduino.h>
#include <variant.h>

// MISSING PLATFORMIO BOARD SUPPORT FEATURES in AdaFruit Grand Central M4 Express variant.h / variant.cpp
// See serial_gcm4.h for documentation.

// Declare the class objects and hook up sercom interrupt handlers for the ports.
// Each port has 4 lines that are all interrupt capable; all interrupts get sent to the same class object handler
// Sercoms are out of order with respect to the SerialX numbering.

#if defined( ARDUINO_GRAND_CENTRAL_M4 )

Uart Serial2( &SERCOM_SERIAL2, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX ) ;

void SERCOM4_0_Handler()
{
  Serial2.IrqHandler();
}
void SERCOM4_1_Handler()
{
  Serial2.IrqHandler();
}
void SERCOM4_2_Handler()
{
  Serial2.IrqHandler();
}
void SERCOM4_3_Handler()
{
  Serial2.IrqHandler();
}

Uart Serial3( &SERCOM_SERIAL3, PIN_SERIAL3_RX, PIN_SERIAL3_TX, PAD_SERIAL3_RX, PAD_SERIAL3_TX ) ;

void SERCOM1_0_Handler()
{
  Serial3.IrqHandler();
}
void SERCOM1_1_Handler()
{
  Serial3.IrqHandler();
}
void SERCOM1_2_Handler()
{
  Serial3.IrqHandler();
}
void SERCOM1_3_Handler()
{
  Serial3.IrqHandler();
}

Uart Serial4( &SERCOM_SERIAL4, PIN_SERIAL4_RX, PIN_SERIAL4_TX, PAD_SERIAL4_RX, PAD_SERIAL4_TX ) ;

void SERCOM5_0_Handler()
{
  Serial4.IrqHandler();
}
void SERCOM5_1_Handler()
{
  Serial4.IrqHandler();
}
void SERCOM5_2_Handler()
{
  Serial4.IrqHandler();
}
void SERCOM5_3_Handler()
{
  Serial4.IrqHandler();
}

#endif
