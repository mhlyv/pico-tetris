#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"

#include "command_buffer.h"

// buffer for storing commands
static volatile uint8_t command_buffer[COMMAND_BUFFER_SIZE] = {0};

// pointer for the next write
static volatile uint8_t *command_write = command_buffer;

// pointer for the next read
static volatile uint8_t *command_read = command_buffer;

// flag for locking the buffer (in case of using the buffer in interrupts)
static volatile bool lock = false;

// write a command to the buffer
// return true if the write was successful, false otherwise
bool command_buffer_write(uint8_t command) {
	bool success = false;
	if (!lock) {
		lock = true;
		success = true;

		// check if the command is valid
		bool valid = command == LEFT_CMD || command == RIGHT_CMD ||
			command == ROTATE_CW_CMD || command == ROTATE_CCW_CMD ||
			command == RESET_CMD || command == DROP_CMD;

		if (valid) {
			*command_write = command;
			command_write++;

			// jump to the beginning if the end was reached
			if (command_write == command_buffer + COMMAND_BUFFER_SIZE) {
				command_write = command_buffer;
			}
		}

		lock = false;
	}
	return success;
}

// read a command to the buffer
// return true if the read was successful, false otherwise
uint8_t command_buffer_read() {
	uint8_t command = 0;
	if (!lock) {
		lock = true;

		command = *command_read;

		// jump to the next if the current one is not 0
		if (command != 0) {
			*command_read = 0;
			command_read++;

			// jump to the beginning if the end was reached
			if (command_read == command_buffer + COMMAND_BUFFER_SIZE) {
				command_read = command_buffer;
			}
		}

		lock = false;
	}
	return command;
}
