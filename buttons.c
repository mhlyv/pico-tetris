#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "buttons.h"
#include "command_buffer.h"
#include "tetris_uart.h"

// last known state of the buttons
// bits:
// 0, 0, 0, 0, left, down, up, right
static volatile uint8_t button_state = 0;

// flag marking if the buttons are ready to be sampled
static volatile bool BUTTONS_READY = false;

// repeating timer structure for the buttons_ready_callback()
static struct repeating_timer buttons_timer;

// returns if the buttons are ready to be sampled
bool buttons_ready() {
	return BUTTONS_READY;
}

// set the flag every <BUTTON_SAMPLE_RATE> ms
static bool buttons_ready_callback(struct repeating_timer *t) {
	(void)t;
	BUTTONS_READY = true;
	return true;
}

// initialize buttons
void buttons_init() {
	gpio_init(LEFT_BUTTON_PIN);
	gpio_set_dir(LEFT_BUTTON_PIN, GPIO_IN);
	gpio_pull_up(LEFT_BUTTON_PIN);

	gpio_init(DOWN_BUTTON_PIN);
	gpio_set_dir(DOWN_BUTTON_PIN, GPIO_IN);
	gpio_pull_up(DOWN_BUTTON_PIN);

	gpio_init(UP_BUTTON_PIN);
	gpio_set_dir(UP_BUTTON_PIN, GPIO_IN);
	gpio_pull_up(UP_BUTTON_PIN);

	gpio_init(RIGHT_BUTTON_PIN);
	gpio_set_dir(RIGHT_BUTTON_PIN, GPIO_IN);
	gpio_pull_up(RIGHT_BUTTON_PIN);

	// start timer
	if (!add_repeating_timer_ms(BUTTON_SAMPLE_RATE, buttons_ready_callback,
				NULL, &buttons_timer)) {
		uart_puts(UART_ID, "Couldn't start repeating timer for buttons\r\n");
	}
}

// return the state of the buttons which have changed
uint8_t buttons_read() {
	BUTTONS_READY = false;
	uint8_t state = 0;
	uint8_t changed = 0;

	// read from pins
	state |= (!gpio_get(LEFT_BUTTON_PIN)) << 3;
	state |= (!gpio_get(DOWN_BUTTON_PIN)) << 2;
	state |= (!gpio_get(UP_BUTTON_PIN)) << 1;
	state |= (!gpio_get(RIGHT_BUTTON_PIN));

	// A button has changed if it went from low to high, all other transitions
	// are ignored. This means that long button presses don't trigger events
	// repeatedly, but it's much simpler this way.
	changed = state & (state ^ button_state);

	// update state
	button_state = state;

	return changed;
}

// read the state of the buttons, write a command to the global command buffer
// if an action was triggered
void buttons_write_to_buffer() {
	switch (buttons_read()) {
		case RIGHT_BUTTON:
			command_buffer_write(RIGHT_CMD);
			break;
		case UP_BUTTON:
			command_buffer_write(ROTATE_CW_CMD);
			break;
		case DOWN_BUTTON:
			command_buffer_write(ROTATE_CCW_CMD);
			break;
		case LEFT_BUTTON:
			command_buffer_write(LEFT_CMD);
			break;
		case DROP_BUTTON:
			command_buffer_write(DROP_CMD);
			break;
		default:
			break;
	}
}
