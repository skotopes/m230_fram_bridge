#include <stdint.h>
#include "uart.h"
#include "spi.h"

#define FRAM_WREN	0b00000110	// Set Write Enable Latch
#define FRAM_WRDI	0b00000100	// Reset Write Enable Latch
#define FRAM_RDSR	0b00000101	// Read Status Register
#define FRAM_WRSR	0b00000001	// Write Status Register
#define FRAM_READ	0b00000011	// Read Memory Code
#define FRAM_WRITE	0b00000010	// Write Memory Code

#define FRAM_BUFFER 32
#define FRAM_WRITE_BLOCK_SIZE 16

int main() {
	// Internal watchdog: disable
	WDTCTL = WDTPW + WDTHOLD;

	// Oscillator configuration: start with failsafe DSO settings
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
	// Enable XT2
	BCSCTL3 |= XT2S_2; // 3-16 mhz range
	BCSCTL1 &= ~XT2OFF; // enable xt2
	// Poke XT2 system and wait till it ready
	do {
		IFG1 &= ~OFIFG;
		__delay_cycles(50000); // 50ms
	} while (IFG1 & OFIFG);
	// Switch main and submain clock to xt2
	BCSCTL2 |= SELM_2 + DIVM_0;	// MCLK uses XT2
	BCSCTL2 |= SELS + DIVS_0;	// SMCLK uses XT2

	// Red led
	P_DIR(P5) |= BIT4; // Direction output
	P_REN(P5) |= BIT4; // Enable pull-up for P1
	P_OUT(P5) |= BIT4; // Set high

	// External watchdog on P5.5, no pull-up
	P_DIR(P5) |= BIT5; // Direction output
	P_OUT(P5) |= BIT5; // Set to high

	// FRAM WP
	P_DIR(P2) |= BIT7; // Direction output
	P_REN(P2) |= BIT7; // Enable pull-up resistor
	P_OUT(P2) |= BIT7; // Set to high

	// Watchdog timer
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_1 + MC_1 + ID_3;	// TASSEL_1: use ACLK, MC_1: upcount, ID_3: divide by 8
	TA0CCR0 = 2048;						// 0.5s

	// UART
	uart_init();
	spi_init();

	__enable_interrupt();

	while (1) {
		uint16_t fram_addr=0;
		uint16_t fram_size=0;
		uint8_t fram_buffer[FRAM_BUFFER];
		uint8_t fram_rx_size;
		uint8_t data_crc;
		char c = uart_getc();
		switch (c) {
			case 'P':
				uart_putc('P');
				break;
			case 'R':
				// address and size to read
				fram_addr |= (uart_getc() << 8);
				fram_addr |= (uart_getc() << 0);
				fram_size |= (uart_getc() << 8);
				fram_size |= (uart_getc() << 0);
				// read fram memory
				data_crc=0;
				while (fram_size != 0) {
					uint8_t block_crc=0;
					// Reqest FRAM read from address
					fram_buffer[0] = FRAM_READ;
					fram_buffer[1] = (fram_addr >> 8) & 0x1f;
					fram_buffer[2] = fram_addr & 0xFF;
					fram_rx_size = (fram_size < FRAM_BUFFER) ? fram_size : FRAM_BUFFER;
					// Transfer data from FRAM
					spi_xfer_bytes(fram_buffer, 3, fram_buffer, fram_rx_size);
					// notify about blocksize
					uart_putc(fram_rx_size);
					for (uint8_t i=0; i < fram_rx_size; i++) {
						uart_putc(fram_buffer[i]);
						data_crc += fram_buffer[i];
						block_crc += fram_buffer[i];
						fram_addr++;
						fram_size--;
					}
					uart_putc(block_crc);
				}
				// END read
				uart_putc(data_crc);
				uart_putc('R');
				break;
			case 'W':
				// address and size to write
				fram_addr |= (uart_getc() << 8);
				fram_addr |= (uart_getc() << 0);
				fram_size |= (uart_getc() << 8);
				fram_size |= (uart_getc() << 0);
				// Unprotect
				fram_buffer[0] = FRAM_WRSR;
				fram_buffer[1] = 0;
				spi_xfer_bytes(fram_buffer, 2, 0, 0);
				// Write fram memory
				data_crc=0;
				while (fram_size != 0) {
					// Enable write
					fram_buffer[0] = FRAM_WREN;
					spi_xfer_bytes(fram_buffer, 1, 0, 0);
					uint8_t block_crc=0;
					fram_rx_size = (fram_size < FRAM_WRITE_BLOCK_SIZE) ? fram_size : FRAM_WRITE_BLOCK_SIZE;
					fram_buffer[0] = FRAM_WRITE;
					fram_buffer[1] = (fram_addr >> 8) & 0x1f;
					fram_buffer[2] = fram_addr & 0xFF;
					uart_putc(fram_rx_size);
					for (uint8_t i=0; i < fram_rx_size; i++) {
						fram_buffer[3+i] = uart_getc();
						data_crc += fram_buffer[3+i];
						block_crc += fram_buffer[3+i];
						fram_addr++;
						fram_size--;
					}
					if (uart_getc() != block_crc) {
						uart_putc('C');
						WDTCTL = 0xDEAD;
					}
					spi_xfer_bytes(fram_buffer, 3+fram_rx_size, 0, 0);
					uart_putc('O');
				}
				uart_putc(data_crc);
				uart_putc('W');
				break;
			case 'E':
				// address and size to write
				fram_addr |= (uart_getc() << 8);
				fram_addr |= (uart_getc() << 0);
				fram_size |= (uart_getc() << 8);
				fram_size |= (uart_getc() << 0);
				// Unprotect
				fram_buffer[0] = FRAM_WRSR;
				fram_buffer[1] = 0;
				spi_xfer_bytes(fram_buffer, 2, 0, 0);
				// Write fram memory
				while (fram_size != 0) {
					// Enable write
					fram_buffer[0] = FRAM_WREN;
					spi_xfer_bytes(fram_buffer, 1, 0, 0);
					// Write
					fram_rx_size = (fram_size < FRAM_WRITE_BLOCK_SIZE) ? fram_size : FRAM_WRITE_BLOCK_SIZE;
					fram_buffer[0] = FRAM_WRITE;
					fram_buffer[1] = (fram_addr >> 8) & 0x1f;
					fram_buffer[2] = fram_addr & 0xFF;
					for (uint8_t i=0; i < fram_rx_size; i++) {
						fram_buffer[3+i] = 0;
						fram_addr++;
						fram_size--;
					}
					spi_xfer_bytes(fram_buffer, 3+fram_rx_size, 0, 0);
				}
				uart_putc('E');
				break;
		}
	}

	return 0;
}

__attribute__((__interrupt__(TIMER0_A0_VECTOR)))
void TIMER0_A0_ISR(void)
{
	P_OUT(P5) ^= BIT4; // flip led
	P_OUT(P5) ^= BIT5; // pet watchdog with a big dildo in the anus
}
