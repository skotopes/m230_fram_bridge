#include <stdint.h>
#include "uart.h"
#include "spi.h"

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

	// P2.4 uart RX hack
	P_DIR(P2) |= BIT4; // Direction output
	// P_REN(P2) |= BIT4; // Enable pull-up
	P_OUT(P2) |= BIT4; // Set to HIGH
	// P_OUT(P2) &= ~BIT4; // Set to LOW

	// Watchdog timer
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_1 + MC_1 + ID_3;	// TASSEL_1: use ACLK, MC_1: upcount, ID_3: divide by 8
	TA0CCR0 = 2048;						// 0.5s

	// UART
	uart_init();
	spi_init();

	__enable_interrupt();

	uart_puts("Hi\r\n");
	while (1) {
		uint16_t fram_addr=0;
		uint16_t fram_size=0;
		char c = uart_getc();
		switch (c) {
			case 'P':
				uart_putc('P');
				break;
			case 'R':
				fram_addr |= (uart_getc() << 8);
				fram_addr |= (uart_getc() << 0);
				fram_size |= (uart_getc() << 8);
				fram_size |= (uart_getc() << 0);
				break;
			case 'W':
				fram_addr |= (uart_getc() << 8);
				fram_addr |= (uart_getc() << 0);
				fram_size |= (uart_getc() << 8);
				fram_size |= (uart_getc() << 0);
				break;
		}
	}

	return 0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	P_OUT(P5) ^= BIT4; // flip led
	P_OUT(P5) ^= BIT5; // pet watchdog with a big dildo in the anus
}
