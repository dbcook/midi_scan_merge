#pragma once
#include <LiquidCrystal_I2C.h>
#include "data.h"

void _SpinDie( const char * msg, int val );
void _SpinDie( const __FlashStringHelper * msg, int val);
void _SpinDie( const __FlashStringHelper * msg );
void _SpinDie( const char * msg );

