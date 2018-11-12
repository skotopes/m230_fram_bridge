#ifndef SPI_H
#define SPI_H

#include "config.h"
#include <stdint.h>

void spi_init();

uint8_t spi_xfer_byte(uint8_t data);

#endif