#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/regs/rosc.h"
#include "pico/stdlib.h"

#include "tetris.h"
#include "command_buffer.h"
#include "tetris_uart.h"
// #include "gyro.h"
#include "display.h"

#define UPDATE_RATE 500

struct Tetris tetris;
volatile bool UPDATE = false;

uint8_t tetris_random() {
	return rand();
}

static inline void seed_random_from_rosc() {
	uint32_t random = 0;
	uint32_t random_bit;
	// location to get random bit
	volatile uint32_t *rnd_reg = (uint32_t *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

	for (int i = 0; i < 32; i++) {
		while (true) {
			random_bit = (*rnd_reg) & 1;
			if (random_bit != ((*rnd_reg) & 1)) {
				break;
			}
		}
		random = (random << 1) | random_bit;
	}
	srand(random);
}

bool update_callback(struct repeating_timer *t) {
	UPDATE = true;
	return true;
}

void update() {
	UPDATE = false;

#ifdef TETRIS_UART_H
	tetris_uart_print(&tetris);
	uart_puts(UART_ID, "\r\n");
#endif // TETRIS_UART_H

#ifdef DISPLAY_H
	// display_clear(0xF800);
	display_tetris(&tetris);
#endif // DISPLAY_H

	tetris_update(&tetris);
}

int main() {
	seed_random_from_rosc();
	stdio_init_all();

#ifdef TETRIS_UART_H
	tetris_uart_init();
#endif // TETRIS_UART_H

#ifdef GYRO_H
	gyro_init();
#endif // GYRO_H

#ifdef DISPLAY_H
	display_init();
#endif // DISPLAY_H

	tetris_init(&tetris);

	// start updating the game every 500 ms
	struct repeating_timer update_timer;
	if (!add_repeating_timer_ms(UPDATE_RATE, update_callback,
				NULL, &update_timer)) {
#ifdef TETRIS_UART_H
		uart_puts(UART_ID, "Couldn't start repeating timer\r\n");
#endif // TETRIS_UART_H
	}

	while (true) {
#ifdef TETRIS_UART_H
		tetris_uart_handle_rx();
#endif // TETRIS_UART_H

#ifdef GYRO_H
		if (gyro_is_ready()) {
			gyro_write_to_buffer();
		}
#endif // GYRO_H

		if (UPDATE) {
			update();
		}
		switch (command_buffer_read()) {
			case 0:
				break;
			case RESET_CMD:
				tetris_init(&tetris);
#ifdef GYRO_H
				gyro_reset();
#endif // GYRO_H
				break;
			case LEFT_CMD:
				tetris_move_tetromino_left(&tetris);
				break;
			case RIGHT_CMD:
				tetris_move_tetromino_right(&tetris);
				break;
			case ROTATE_CW_CMD:
				tetris_rotate_cw(&tetris);
				break;
			case ROTATE_CCW_CMD:
				tetris_rotate_ccw(&tetris);
				break;
			default:
				break;
		}
		// sleep_ms(10);
	}
}
