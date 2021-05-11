#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "tetris.h"

#define DISPLAY_WIDTH 80
#define DISPLAY_HEIGHT 160
#define DISPLAY_BLOCK_SIZE 8
#define DISPLAY_FONT_SIZE 8

#define DISPLAY_SPI spi1
#define DISPLAY_SPI_FREQ (30 * MHZ)
#define DISPLAY_SCK_PIN 10
#define DISPLAY_TX_PIN 11
#define DISPLAY_CS_PIN 13
#define DISPLAY_DC_PIN 14
#define DISPLAY_RST_PIN 15

#define WHITE_COLOR 0xFFFF
#define BLACK_COLOR 0x0
#define RED_COLOR 0xF800
#define BLUE_COLOR 0x001F
#define GREEN_COLOR 0x07E0

void display_init();
void display_clear(uint16_t color);
void display_tetris(struct Tetris *tetris);
void display_draw_char(uint8_t x, uint8_t y, char c);
uint8_t display_draw_string(const char *str, uint8_t row);

#endif // DISPLAY_H
