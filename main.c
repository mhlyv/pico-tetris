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
#include "buttons.h"

#define UPDATE_RATE 500

// the state of the tetris game
static struct Tetris tetris;

// flag if update() needs to be called
static volatile bool UPDATE = false;

// flag if the game is over
static volatile bool GAME_OVER = false;

// repeating timer structure for update_callback()
static struct repeating_timer update_timer;

// set the forward declared random value function for the tetris library
uint8_t tetris_random() {
	return rand();
}

// set a random seed using the ROSC timer
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

// set the UPDATE flag every <UPDATE_RATE> ms
static bool update_callback(struct repeating_timer *t) {
	(void)t;
	UPDATE = true;
	return true;
}

// update
static void update() {
	UPDATE = false;

	// check if the game is over
	GAME_OVER = tetris_update(&tetris);
}

static inline void refresh_game_state() {
	// print tetris state on uart
	tetris_uart_print(&tetris);
	uart_puts(UART_ID, "\r\n");

	// display game state
	display_tetris(&tetris);
}

// start updating the game
static void start_update() {
	// start updating the game every 500 ms
	if (!add_repeating_timer_ms(UPDATE_RATE, update_callback,
				NULL, &update_timer)) {
		uart_puts(UART_ID, "Couldn't start repeating timer\r\n");
	}
}

static inline void show_game_over() {
	// display stats
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
}

// stop updating the game
static void stop_update() {
	cancel_repeating_timer(&update_timer);
}

int main() {
	// initialize everything
	seed_random_from_rosc();
	stdio_init_all();
	tetris_uart_init();
	gyro_init();
	display_init();
	buttons_init();
	tetris_init(&tetris);

	start_update();

	// main event loop
	while (true) {
		tetris_uart_handle_rx();

		if (GAME_OVER) {
			// if the game is over display statistics, and wait for a button
			// press to reset and restart the game

			// stop updating the game
			stop_update();

			// add score and display game over screen
			toplist_add(tetris.score);
			show_game_over();

			// restart on button press or reset command
			while (!buttons_read() && command_buffer_read() != RESET_CMD) {
				tetris_uart_handle_rx();
			}

			// reset
			tetris_init(&tetris);
			gyro_reset();
			GAME_OVER = false;

			// start updating the game
			start_update();
		} else {
			// change to true if the game state changed, so it needs to be
			// redrawn
			volatile bool need_refresh = false;

			// if the game is still running, poll the peripherials if they
			// are ready to be sampled or updated, then handle the next command
			// in the command buffer

			// check gyroscope
			if (gyro_is_ready()) {
				// get actions from gyroscope, write commands to the command
				// buffer if there are any events
				gyro_write_to_buffer();
			}

			// check buttons
			if (buttons_ready()) {
				// get actions from the buttons, write commands to the command
				// buffer if there are any events
				buttons_write_to_buffer();
			}

			// check if an update is needed
			if (UPDATE) {
				// update
				update();
				need_refresh = true;
			}

			// read the next command from the command buffer and handle it
			switch (command_buffer_read()) {
				case RESET_CMD:
					tetris_init(&tetris);
					gyro_reset();
					break;
				case LEFT_CMD:
					tetris_move_tetromino_left(&tetris);
					need_refresh = true;
					break;
				case RIGHT_CMD:
					tetris_move_tetromino_right(&tetris);
					need_refresh = true;
					break;
				case ROTATE_CW_CMD:
					tetris_rotate_cw(&tetris);
					need_refresh = true;
					break;
				case ROTATE_CCW_CMD:
					tetris_rotate_ccw(&tetris);
					need_refresh = true;
					break;
				case DROP_CMD:
					tetris_drop(&tetris);
					need_refresh = true;
					break;
				case GYRO_TOGGLE_CMD:
					gyro_toggle();
					break;
				default:
					break;
			}

			if (need_refresh) {
				refresh_game_state();
			}
		}
	}
}
