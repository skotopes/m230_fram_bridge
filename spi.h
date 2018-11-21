#ifndef SPI_H
#define SPI_H

#include "config.h"
#include <stdint.h>

void spi_init();

void spi_xfer_bytes(uint8_t *tx_buff, uint8_t tx_xfer, uint8_t *rx_buff, uint8_t rx_xfer);

#endif