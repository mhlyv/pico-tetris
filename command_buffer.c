#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"

#include "command_buffer.h"

volatile static uint8_t command_buffer[COMMAND_BUFFER_SIZE] = {0};
volatile static uint8_t *command_write = command_buffer;
volatile static uint8_t *command_read = command_buffer;
volatile static bool lock = false;

bool command_buffer_write(uint8_t command) {
	bool success = false;
	if (!lock) {
		lock = true;
		success = true;

		bool valid = command == LEFT_CMD || command == RIGHT_CMD ||
			command == ROTATE_CW_CMD || command == ROTATE_CCW_CMD ||
			command == RESET_CMD || command == DROP_CMD;

		if (valid) {
			*command_write = command;
			command_write++;
			if (command_write == command_buffer + COMMAND_BUFFER_SIZE) {
				command_write = command_buffer;
			}
		}

		lock = false;
	}
	return success;
}

uint8_t command_buffer_read() {
	uint8_t command = 0;
	if (!lock) {
		lock = true;

		command = *command_read;

		// jump to the next if the current one is not 0
		if (command != 0) {
			*command_read = 0;
			command_read++;
			if (command_read == command_buffer + COMMAND_BUFFER_SIZE) {
				command_read = command_buffer;
			}
		}

		lock = false;
	}
	return command;
}
