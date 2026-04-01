#ifndef UART_GS_H
#define UART_GS_H

#include <stdint.h>
#include <stddef.h>

void uart_gs_init(void);
int uart_gs_send(const uint8_t *data, size_t len);
int uart_gs_read(uint8_t *data, size_t max_len);

#endif