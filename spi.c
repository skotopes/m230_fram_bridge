#include "spi.h"
#include "uart.h"

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

void spi_xfer_bytes(uint8_t *tx_buff, uint8_t tx_size, uint8_t *rx_buff, uint8_t rx_size)
{
	// CS low
	P_OUT(SPI_P) &= ~SPI_CS;
	// Transmit
	while (tx_size != 0) {
		UC_TXBUF(SPI_UC) = *tx_buff;
		while (!(SPI_IFG & UC_TXIFG(SPI_UC)));
		tx_buff++;
		tx_size--;
	}
	// Recieve
	while (rx_size != 0) {
		UC_TXBUF(SPI_UC) = 0;
		while (!(SPI_IFG & UC_RXIFG(SPI_UC)));
		*rx_buff = UC_RXBUF(SPI_UC);
		rx_buff++;
		rx_size--;
	}
	// CS high
	P_OUT(SPI_P) |= SPI_CS;
}
