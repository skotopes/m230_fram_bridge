#ifndef UART_H
#define UART_H

#include "config.h"
#include <stdint.h>

void uart_init();

void uart_putc(char c);

void uart_puts(const char *str);

char uart_getc();

#endif