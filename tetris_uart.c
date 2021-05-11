#include <stdbool.h>

#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "tetris.h"
#include "tetris_uart.h"

// initialize uart
void tetris_uart_init() {
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	uart_init(UART_ID, UART_BAUD_RATE);
	
	uart_set_hw_flow(UART_ID, false, false); // disable CTS/RTS
	uart_set_fifo_enabled(UART_ID, true);
}

// print the tetris board
void tetris_uart_print(struct Tetris *tetris) {
	for (uint8_t i = 0; i < BOARD_H; i++) {
		for (uint8_t j = 0; j < BOARD_W; j++) {
			uart_putc(UART_ID, tetris_get_board_block(tetris, j, i) ? '#' : '.');
			uart_putc(UART_ID, ' ');
		}
		uart_puts(UART_ID, "\r\n");
	}
}

// read commands from the uart, and write them to the command buffer
void tetris_uart_handle_rx() {
	while (uart_is_readable(UART_ID)) {
		while (!command_buffer_write(uart_getc(UART_ID))) {
			tight_loop_contents();
		}
	}
}
