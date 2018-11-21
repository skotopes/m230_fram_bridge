#ifndef CPU_H
#define CPU_H

#define PRIMITIVE_CAT(base, ...) base ## __VA_ARGS__

// UC Registers
#define UC_CTL0(base) PRIMITIVE_CAT(base, CTL0)
#define UC_CTL1(base) PRIMITIVE_CAT(base, CTL1)
#define UC_BR0(base) PRIMITIVE_CAT(base, BR0)
#define UC_BR1(base) PRIMITIVE_CAT(base, BR1)
#define UC_MCTL(base) PRIMITIVE_CAT(base, MCTL)

#define UC_TXIFG(base) PRIMITIVE_CAT(base, TXIFG)
#define UC_TXBUF(base) PRIMITIVE_CAT(base, TXBUF)
#define UC_TXIE(base) PRIMITIVE_CAT(base, TXIE)

#define UC_RXIFG(base) PRIMITIVE_CAT(base, RXIFG)
#define UC_RXBUF(base) PRIMITIVE_CAT(base, RXBUF)
#define UC_RXIE(base) PRIMITIVE_CAT(base, RXIE)

#define UC_STAT(base) PRIMITIVE_CAT(base, STAT)
#define UC_IFG(base) PRIMITIVE_CAT(base, IFG)

// Port Registers
#define P_SEL(port) PRIMITIVE_CAT(port, SEL)
#define P_REN(port) PRIMITIVE_CAT(port, REN)
#define P_DIR(port) PRIMITIVE_CAT(port, DIR)
#define P_OUT(port) PRIMITIVE_CAT(port, OUT)

// USCI registers calculator
#define USCI_DIV_INT              (XT2_CLK/UART_BR)
#define USCI_BR0_VAL              (USCI_DIV_INT & 0x00FF)
#define USCI_BR1_VAL              ((USCI_DIV_INT >> 8) & 0xFF)

#define USCI_DIV_FRAC_NUMERATOR   (XT2_CLK - (USCI_DIV_INT*UART_BR))
#define USCI_DIV_FRAC_NUM_X_8     (USCI_DIV_FRAC_NUMERATOR*8)
#define USCI_DIV_FRAC_X_8         (USCI_DIV_FRAC_NUM_X_8/UART_BR)

#if (((USCI_DIV_FRAC_NUM_X_8-(USCI_DIV_FRAC_X_8*UART_BR))*10)/UART_BR < 5)
#define USCI_BRS_VAL              (USCI_DIV_FRAC_X_8<< 1)
#else
#define USCI_BRS_VAL              ((USCI_DIV_FRAC_X_8+1)<< 1)
#endif

#endif