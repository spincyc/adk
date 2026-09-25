#pragma once

// ADK: declare the parts of a circuit, call adk::setup () once, and
// adk::update () at the top of every loop ().

#if __cplusplus < 202302L
#error "ADK needs C++23. Choose the ADK Mega 2560 board, as Getting started explains."
#endif

#include "adk/object.h"
#include "adk/board.h"
#include "adk/every.h"
#include "adk/timer.h"
#include "adk/containers.h"
#include "adk/print.h"

#include "adk/digital.h"
#include "adk/analog.h"
#include "adk/debouncer.h"

#include "adk/led.h"
#include "adk/rgb_led.h"
#include "adk/buzzer.h"
#include "adk/speaker.h"
#include "adk/relay.h"
#include "adk/motor.h"
#include "adk/servo.h"
#include "adk/stepper.h"

#include "adk/button.h"
#include "adk/keypad.h"
#include "adk/rotary_encoder.h"
#include "adk/joystick.h"
#include "adk/thermistor.h"
#include "adk/ultrasonic.h"
#include "adk/dht11.h"
#include "adk/ir_receiver.h"
#include "adk/ds18b20.h"

#include "adk/shift_register.h"
#include "adk/segments.h"
#include "adk/seven_segment.h"
#include "adk/four_digit_display.h"
#include "adk/font.h"
#include "adk/lcd.h"
#include "adk/led_matrix.h"

#include "adk/i2c.h"
#include "adk/rtc.h"
#include "adk/mpu6050.h"

#include "adk/spi.h"
#include "adk/rfid.h"

#include "adk/radio.h"
#include "adk/fm_radio.h"
