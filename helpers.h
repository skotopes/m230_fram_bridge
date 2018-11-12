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

// Port Registers
#define P_SEL(port) PRIMITIVE_CAT(port, SEL)
#define P_REN(port) PRIMITIVE_CAT(port, REN)
#define P_DIR(port) PRIMITIVE_CAT(port, DIR)
#define P_OUT(port) PRIMITIVE_CAT(port, OUT)


#endif