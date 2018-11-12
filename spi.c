#include "spi.h"

void spi_init() {
	// Configure pins
	P_DIR(SPI_P) |= SPI_CS;
	P_OUT(SPI_P) |= SPI_CS;
	P_SEL(SPI_P) |= SPI_SI + SPI_SO + SPI_CK;
	// Configure USCI
	UC_CTL1(SPI_UC) = UCSWRST;
	UC_CTL0(SPI_UC) |= UCCKPH + UCMSB + UCMST + UCSYNC;
	UC_CTL1(SPI_UC) |= UCSSEL_2;

	UC_CTL1(SPI_UC) &= ~UCSWRST;
}

uint8_t spi_xfer_byte(uint8_t data)
{
	UC_TXBUF(SPI_UC) = data;
	// wait for TX
	while (!(IFG2 & UC_TXIFG(SPI_UC)));

	return UC_RXBUF(SPI_UC);
}
