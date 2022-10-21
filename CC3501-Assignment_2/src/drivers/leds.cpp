#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "WS2812.pio.h" // This header file gets produced during compilation from the WS2812.pio file

#include "drivers/leds.h" // includes elements from leds.h


#define LED_PIN 14 // hardcoded for board design
static int led_order_data[12];

// initialise the LEDS
void init_leds()
{
    // Initialise PIO0 to control the LED chain
    uint pio_program_offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, pio_program_offset, LED_PIN, 800000, false);
    // initialise empty colour arrays, should optimise!!!! 
    //  uint32_t led_data [1];
    // led_data[0] = (0 << 24) | (0 << 16) | (0 << 8);
    int i;
    for (i = 0; i < 12; i++)
    {
    led_order_data[i] = 0;
    }
}


// sets a singular led and turns it on
void set_led_colour(int LED, uint8_t red, uint8_t green, uint8_t blue) 
{
    // add an array to save values 
    // how do i keep the array on each function call
    // NOTE that current set up can only update 1 LED at a time, potentially break out later argument to handle updating all at once

    led_order_data[LED -1] = (red << 24) | (green << 16) | (blue << 8);
    
    // 12 blocks that repeatly pull the colour
    int i;
    for (i = 0; i < 12; i++ )
    {
       pio_sm_put_blocking(pio0, 0, led_order_data[i]); 
    }
}

// sets the LEDs on, doesnt set any new values
void set_leds_on()
{   
    int i;
    for (i = 0; i < 12; i++ )
    {
       pio_sm_put_blocking(pio0, 0, led_order_data[i]); 
    }
}

//
void set_leds_off()
{   
    int i;
    for (i = 0; i < 12; i++ )
    {
       pio_sm_put_blocking(pio0, 0, 0); 
    }
}

void clear_leds()
{
    int i;
    for (i = 0; i < 12; i++ )
    {
       pio_sm_put_blocking(pio0, 0, 0); 
    }
    int j;
    for (j = 0; j < 12; j++)
    {
    led_order_data[j] = 0;
    }
}


