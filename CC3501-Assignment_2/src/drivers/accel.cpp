#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "drivers/accel.h" // includes elements from accel.h
#include "drivers/logging/style1.h"


#define I2C_INSTANCE i2c0
#define accel_SDA_PIN 16
#define accel_SCL_PIN 17
// #define I2C_ADDRESS_DUMMY 0b0001111 // more for my own benfit 
// #define I2C_ADDRESS_DUMMY_OUTPUT 0b00110011 // more for my own benfit 
#define I2C_ADDRESS 0b0011001 //slave address (no read or write info, connected to power supply)



void init_accel()
{
    //init  i2c for 400Hz
    i2c_init(I2C_INSTANCE, 400*1000);
    gpio_set_function(accel_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(accel_SCL_PIN, GPIO_FUNC_I2C);
}

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

bool read_accel_registor(uint8_t reg, uint8_t* data)
{
    read_accel_registors(reg, data, 1);
    return true; 
}

bool read_accel_registors(uint8_t reg, uint8_t* data, size_t length) //coppied from class notes
{
    if (length > 1) {
        reg |= (1 << 7); // Must assert the MSB of the registor address to enable multi register read. Section 6.1.1
    }
    // tell the device which address to read?
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

bool convert_to_axis_data(uint8_t* raw_data, int16_t* axis_data, float* g_force)
{
   axis_data[0] = (int16_t)(raw_data[0] | (raw_data[1] << 8 )) >> 6;  
   axis_data[1] = (int16_t)(raw_data[2] | (raw_data[3] << 8 )) >> 6;  
   axis_data[2] = (int16_t)(raw_data[4] | (raw_data[5] << 8 )) >> 6;


   g_force[0] = axis_data[0] * 0.004; 
   g_force[1] = axis_data[1] * 0.004; 
   g_force[2] = axis_data[2] * 0.004; 

   return true;
}
