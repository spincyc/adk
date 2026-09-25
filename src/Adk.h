#pragma once

// ADK: declare the parts of a circuit, call adk::setup () once, and
// adk::update () at the top of every loop ().

#include "adk/object.h"
#include "adk/board.h"
#include "adk/every.h"

#include "adk/digital.h"
#include "adk/analog.h"
#include "adk/debouncer.h"

#include "adk/led.h"
#include "adk/rgb_led.h"
#include "adk/buzzer.h"
#include "adk/speaker.h"

#include "adk/button.h"
#include "adk/keypad.h"
#include "adk/rotary_encoder.h"
#include "adk/joystick.h"
#include "adk/thermistor.h"

#include "adk/shift_register.h"
#include "adk/segments.h"
#include "adk/seven_segment.h"
#include "adk/four_digit_display.h"
