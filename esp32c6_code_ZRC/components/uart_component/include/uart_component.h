#ifndef UART_ZUMO_H
#define UART_ZUMO_H

#include "driver/uart.h"

#define UART_PORT (UART_NUM_1) // Recuerda: C6 no tiene UART2
#define BUF_SIZE (256)

void init_uart(void);

#endif