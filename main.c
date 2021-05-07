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
#include "gyro.h"
#include "display.h"
#include "toplist.h"

#define UPDATE_RATE 500

struct Tetris tetris;
volatile bool UPDATE = false;
volatile bool GAME_OVER = false;

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

	tetris_uart_print(&tetris);
	uart_puts(UART_ID, "\r\n");

	display_tetris(&tetris);

	GAME_OVER = tetris_update(&tetris);
}

int main() {
	seed_random_from_rosc();

	stdio_init_all();
	tetris_uart_init();
	gyro_init();
	display_init();
	tetris_init(&tetris);

	// start updating the game every 500 ms
	struct repeating_timer update_timer;
	if (!add_repeating_timer_ms(UPDATE_RATE, update_callback,
				NULL, &update_timer)) {
		uart_puts(UART_ID, "Couldn't start repeating timer\r\n");
	}

	while (true) {
		tetris_uart_handle_rx();

		if (GAME_OVER) {
			toplist_add(tetris.score);
			uint8_t display_y = 9;

			uart_puts(UART_ID, "Game Over!\r\n");
			display_y += display_draw_string("Game Over!", display_y);

			char buf[32];
			sprintf(buf, "score: %u\r\n", tetris.score);
			uart_puts(UART_ID, buf);
			display_y += display_draw_string(buf, display_y);

			uart_puts(UART_ID, "toplist:\r\n");
			display_y += display_draw_string("toplist:", display_y);

			for (uint8_t i = 0; i < TOPLIST_LENGTH; i++) {
				sprintf(buf, "%u. %u\r\n", i + 1, toplist_read(i));
				uart_puts(UART_ID, buf);
				display_y += display_draw_string(buf, display_y);
			}

			sleep_ms(5000);

			// reset
			tetris_init(&tetris);
			gyro_reset();
			GAME_OVER = false;
		} else {
			if (gyro_is_ready()) {
				gyro_write_to_buffer();
			}

			if (UPDATE) {
				update();
			}
			switch (command_buffer_read()) {
				case 0:
					break;
				case RESET_CMD:
					tetris_init(&tetris);
					gyro_reset();
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
				case DROP_CMD:
					tetris_drop(&tetris);
					break;
				default:
					break;
			}
		}
	}
}
