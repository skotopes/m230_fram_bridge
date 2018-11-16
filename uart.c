#include "uart.h"

volatile uint8_t echo = 0;

void uart_init() {
	// Configure pins
	P_SEL(UART_P) |= UART_RX + UART_TX;
	// Set clock source to SMCLK and switch to reset state
	UC_CTL1(UART_UC) |= UCSSEL_2 + UCSWRST;
	// Set baud rate (dividers calculated based on config values)
	UC_BR0(UART_UC) = USCI_BR0_VAL;
	UC_BR1(UART_UC) = USCI_BR1_VAL;
	UC_MCTL(UART_UC) = USCI_BRS_VAL;
	// Clear reset state
	UC_CTL1(UART_UC) &= ~UCSWRST;
}

void uart_putc(char c)
{
	while (!(UC1IFG & UC_TXIFG(UART_UC)));
	UC_TXBUF(UART_UC) = c;
	echo = 1;
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
	// Echo cancelation, discard first symbol that arrives after TX was active
	while (echo) {
		while (!(UC1IFG & UC_TXIFG(UART_UC)));
		while (!(UC1IFG & UC_RXIFG(UART_UC)));
		UC1IFG &= ~UC_RXIFG(UART_UC);
		echo = 0;
	}

	while (!(UC1IFG & UC_RXIFG(UART_UC)));
	UC1IFG &= ~UC_RXIFG(UART_UC);
	return UC_RXBUF(UART_UC);
}
