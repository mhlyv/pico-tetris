#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

#define COMMAND_BUFFER_SIZE 16
#define LEFT_CMD 'h'
#define RIGHT_CMD 'l'
#define ROTATE_CW_CMD 'k'
#define ROTATE_CCW_CMD 'j'
#define RESET_CMD 'r'
#define DROP_CMD 'd'

bool command_buffer_write(uint8_t command);
uint8_t command_buffer_read();

#endif // COMMAND_BUFFER_H
