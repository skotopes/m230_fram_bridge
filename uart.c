#include "uart.h"

void uart_init() {
	// Configure pins
	P_SEL(UART_P) |= UART_RX + UART_TX;
	// Set clock source to SMCLK
	UC_CTL1(UART_UC) |= UCSSEL_2;
	// Set baud rate
	UC_BR0(UART_UC) = 104;			// 1MHz 9600
	UC_BR1(UART_UC) = 0;			// 1MHz 9600
	UC_MCTL(UART_UC) = UCBRS0;		// Modulation UCBRSx = 1
	// Clear reset state
	UC_CTL1(UART_UC) &= ~UCSWRST;
}

void uart_putc(char c)
{
	while (!(IFG2 & UC_TXIFG(UART_UC))) ;
	UC_TXBUF(UART_UC) = c;
}

void uart_puts(const char *str)
{
	while (*str != '\0'){
		uart_putc(*str);
		str++;
	}
}

char uart_getc()
{
	while (!(IFG2 & UC_RXIFG(UART_UC)));
	return UC_RXBUF(UART_UC);
}
