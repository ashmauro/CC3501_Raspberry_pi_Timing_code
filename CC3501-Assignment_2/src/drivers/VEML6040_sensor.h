#include "pico/stdlib.h"

void init_VEML6040();

bool write_registor(uint8_t reg, uint8_t data); 

bool read_registors(uint8_t reg, uint8_t* data, size_t length);