#include "pico/stdlib.h"

void init_accel();

bool write_accel_registor(uint8_t reg, uint8_t data); // changed from bool

bool read_accel_registor(uint8_t reg, uint8_t* data);

bool read_accel_registors(uint8_t reg, uint8_t* data, size_t length);

bool convert_to_axis_data(uint8_t* raw_data, int16_t* axis_data, float* g_force);