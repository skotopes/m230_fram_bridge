#include <stdint.h>
#include "uart.h"
#include "spi.h"

#define FRAM_WREN	0b00000110	// Set Write Enable Latch
#define FRAM_WRDI	0b00000100	// Reset Write Enable Latch
#define FRAM_RDSR	0b00000101	// Read Status Register
#define FRAM_WRSR	0b00000001	// Write Status Register
#define FRAM_READ	0b00000011	// Read Memory Code
#define FRAM_WRITE	0b00000010	// Write Memory Code

#define FRAM_BUFFER 64

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
		uint8_t crc=0;
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
				while (fram_size != 0) {
					// Reqest FRAM read from address
					fram_buffer[0] = FRAM_READ;
					fram_buffer[1] = (fram_addr >> 8) & 0x1f;
					fram_buffer[2] = fram_addr & 0xFF;
					fram_rx_size = (fram_size < FRAM_BUFFER) ? fram_size : FRAM_BUFFER;
					// Xfer
					spi_xfer_bytes(fram_buffer, 3, fram_buffer, fram_rx_size);
					for (uint8_t i=0; i < fram_rx_size; i++) {
						crc += fram_buffer[i];
						uart_putc(fram_buffer[i]);
						fram_addr++;
						fram_size--;
					}
				}
				// END read
				uart_putc(crc);
				uart_putc('R');
				break;
			case 'W':
				// address and size to write
				fram_addr |= (uart_getc() << 8);
				fram_addr |= (uart_getc() << 0);
				fram_size |= (uart_getc() << 8);
				fram_size |= (uart_getc() << 0);
				// write fram memory
				while (fram_size != 0) {
				}
				uart_putc('W');
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
