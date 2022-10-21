#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "drivers/VEML6040_sensor.h"



#define I2C_INSTANCE i2c1
#define SDA_PIN 2
#define SCL_PIN 3
#define I2C_ADDRESS 0x10


// NOT USING THIS JUST PLAYED AROUND 
void init_VEML6040()
{
    //init  i2c for 400Hz
    i2c_init(I2C_INSTANCE, 400*1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
}

// kinda useless, not "just writting" anything
bool write_accel_registor(uint8_t reg, uint8_t data) //coppied from class notes 
{   
    uint8_t buf[2];
    buf[0] = reg; //registor address
    buf[1] = data; // new value

    int bytes_written = i2c_write_blocking(I2C_INSTANCE, I2C_ADDRESS, buf, 2, false);
    if (bytes_written !=2)
    {
        printf("buffer error");
        return false;
    }
    return true;
}



bool read_registors(uint8_t reg, uint8_t* data, size_t length) //ISSUE: VEML4060 returns 16 bit data, RP2040 i2c SDK notes it handles 8 bits!
{
    if (length > 1) {
        reg |= (1 << 7); // Must assert the MSB of the registor address to enable multi register read. Section 6.1.1
    }
    // tell the device which address to read by writting to it
    if (1 != i2c_write_blocking(I2C_INSTANCE, I2C_ADDRESS, &reg, 1, true))
    {
        printf("registor error, did not select registor");
        return false;
    }

    int bytes_read = i2c_read_blocking(I2C_INSTANCE, I2C_ADDRESS, data, length, false); 
    if (bytes_read != 1) 
    {
        printf("failed to read data");
        return false;
    }
    return true;
}