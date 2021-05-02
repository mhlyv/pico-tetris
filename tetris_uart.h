#ifndef TETRIS_UART_H
#define TETRIS_UART_H

#include <stdbool.h>
#include <stdint.h>

#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "tetris.h"
#include "command_buffer.h"

#define UART_ID uart0
#define UART_IRQ (UART_ID == uart0 ? UART0_IRQ : UART1_IRQ)
#define UART_BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

bool tetris_uart_is_ready();
void tetris_uart_init();
void tetris_uart_print(struct Tetris *tetris);
void tetris_uart_handle_rx();
void tetris_uart_write_to_buffer();

#endif // TETRIS_UART_H
