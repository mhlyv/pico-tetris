#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

#define UP_BUTTON_PIN 16
#define LEFT_BUTTON_PIN 17
#define RIGHT_BUTTON_PIN 18
#define DOWN_BUTTON_PIN 19

#define RIGHT_BUTTON 1
#define UP_BUTTON 2
#define DOWN_BUTTON 4
#define LEFT_BUTTON 8
#define DROP_BUTTON (UP_BUTTON + DOWN_BUTTON)

#define BUTTON_SAMPLE_RATE 70

void buttons_init();
uint8_t buttons_read();
bool buttons_ready();
void buttons_write_to_buffer();

#endif // BUTTONS_H
