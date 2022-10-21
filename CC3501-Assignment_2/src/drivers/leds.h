#include "pico/stdlib.h"
void init_leds();

void set_led_colour(int LED, uint8_t red, uint8_t green, uint8_t blue); // sets colour and turns on a LED

void set_leds_off(); //turns off all LEDs

void set_leds_on(); // turns on all LEDs

void clear_leds(); //reset the LED colour value to 0 (nothing)


