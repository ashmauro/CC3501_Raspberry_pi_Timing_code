#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "WS2812.pio.h" // This header file gets produced during compilation from the WS2812.pio file
#include "drivers/logging/logging.h"
#include "drivers/leds.h"

// uint8_t raw_data[6]; // raw data for the 6 returns of RGB data
// int16_t RGB_data[3]; // RGB stat combined in order to be used for programing 

// choose different LED_PIN based on what your using
#define LED_PIN 5  
// #define LED_PIN 6


int main()
{
        stdio_init_all();// needed for some functions
        init_leds(); //used to set up the LEDS ( same as the pracs)
        gpio_init(LED_PIN); // set up for the IR break beam response
        gpio_set_dir(LED_PIN, false);
        // init_VEML6040();



        for (;;) {
                // MAIN LOOP

                // initial work to test board to PC communications 
                // int i;
                // for (i=1;i<13;i++)
                // {
                //         set_led_colour(i,50,0,0);
                //         sleep_ms(200);
                //         set_led_colour(i,0,50,0);
                //         sleep_ms(200);
                //         set_led_colour(i,0,0,50);
                //         sleep_ms(200);
                //         set_leds_off();
                //         sleep_ms(200);
                //         printf("cheese");
                // }
   
                //uncomment the desired gate's logic, comment out all others.
                // useing letters like "a", "b", "c" etc to identify the gates, since the i2c output will be numbers 

                // START/FINISH GATE LOGIC
                if (gpio_get(LED_PIN) == 0) 
                {
                        printf("a\n");
                        set_led_colour(1,0,100,0);
                        //TODO ADD functionality of i2c to capture the colour at this point, likely use a specific driver 
                        sleep_ms(3000);
                }
                else
                {
                        set_led_colour(1,100,0,0);
                }


                // SECOND GATE LOGIC
                if (gpio_get(LED_PIN) == 0) 
                {
                        printf("b\n");
                        set_led_colour(1,0,100,0);
                        //TODO ADD functionality of i2c to capture the colour at this point, likely use a specific driver 
                        sleep_ms(3000);
                }
                else
                {
                        set_led_colour(1,100,0,0);
                }

                // THIRD GATE LOGIC
                if (gpio_get(LED_PIN) == 0) 
                {
                        printf("c\n");
                        set_led_colour(1,0,100,0); // sets to green when IR breaks
                        //TODO ADD functionality of i2c to capture the colour at this point, likely use a specific driver 
                        sleep_ms(3000);
                }
                else
                {
                        set_led_colour(1,100,0,0); //set to red when nothing is happening 
                }

                sleep_ms(50); // intorduces delay (negative) allows LEDs to work (positive that im okay with losing)
        }
return 0;
}
