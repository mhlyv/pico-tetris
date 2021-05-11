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

// flags for locking the buffer (in case of using the buffer in interrupts)
static volatile bool write_lock = false;
static volatile bool read_lock = false;

// write a command to the buffer
// return true if the write was successful, false otherwise
bool command_buffer_write(uint8_t command) {
	// lock
	write_lock = true;

	bool success = false;
	
	// check if currently reading
	if (!read_lock) {
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
	}

	// unlock
	write_lock = false;
	return success;
}

// read a command to the buffer
// return the command if the read was successful, 0 otherwise
uint8_t command_buffer_read() {
	// lock
	read_lock = true;

	uint8_t command = 0;

	// check if currently writing
	if (!write_lock) {
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
	}

	// unlock
	read_lock = false;
	return command;
}
