#ifndef INIT_H
#define INIT_H

#include <stdio.h>
#include "hardware/i2c.h"
#include "display.h"

#define I2C_SDA_SENSORS 0 // Pino SDA do I2C0
#define I2C_SCL_SENSORS 1 // Pino SCL do I2C0

void initializeSystem();

#endif
