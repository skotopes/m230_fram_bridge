#ifndef CONFIG_H
#define CONFIG_H

#include <msp430f249.h>

// CPU
#define XT2_CLK		(6000000UL)	// in Hz
// UART
#define UART_UC		UCA1		// UCA1
#define UART_IFG	UC1IFG		// UC1IFG for UCA1
#define UART_P		P3			// p3
#define UART_RX		BIT7		// p3.7 UCA1RXD
#define UART_TX		BIT6		// p3.6 UCA1TXD
#define UART_BR		(9600)		// 9600
// SPI
#define SPI_UC		UCB0		// UCB0
#define SPI_IFG		IFG2		// IFG2 for UCB0
#define SPI_P 		P3			// p3
#define SPI_CS		BIT0		// p3.0 UCB0STE
#define SPI_SI		BIT1		// p3.1 UCB0SIMO
#define SPI_SO		BIT2		// p3.2 UCB0SOMI
#define SPI_CK		BIT3		// p3.3 UCB0CLK

#include "helpers.h"

#endif
