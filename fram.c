#include <stdint.h>
#include "uart.h"
#include "spi.h"

int main() {
	// Internal watchdog: disable
	WDTCTL = WDTPW + WDTHOLD;

	// Oscillator configuration: use DCO
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	// Red led
	P_DIR(P5) |= BIT4; // Direction output
	P_REN(P5) |= BIT4; // Enable pull up/down for P1
	P_OUT(P5) |= BIT4; // Set high

	// External watchdog on P5.5, no pull-up
	P_DIR(P5) |= BIT5; // Direction output
	P_OUT(P5) |= BIT5; // Set to high

	// Watchdog timer
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_2 + MC_1 + ID_3; /* TASSEL_2: use SMCLK MC_1: upcount ID_3: divide by 8*/
	TA0CCR0 = 65535;

	// UART
	uart_init();
	spi_init();

	__enable_interrupt();

	uart_puts("Hi\r\n");
	while (1) {
		switch (uart_getc()) {
			case 'P': uart_putc('P'); break;
			case 'R': break;
			case 'W': break;
			default: uart_puts("Invalid opcode.\r\n"); break;
		}
	}
	uart_puts("Bye\r\n");
	return 0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	P_OUT(P5) ^= BIT4; // flip led
	P_OUT(P5) ^= BIT5; // pet watchdog with a big dildo in the anus
}
