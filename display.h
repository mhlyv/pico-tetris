#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"

#define DISPLAY_WIDTH 80
#define DISPLAY_HEIGHT 160
#define DISPLAY_SPI_FREQ (30 * MHZ)

#define DISPLAY_SPI spi1
#define DISPLAY_SCK_PIN 10
#define DISPLAY_TX_PIN 11
#define DISPLAY_CS_PIN 13
#define DISPLAY_DC_PIN 14
#define DISPLAY_RST_PIN 15

#define WHITE_COLOR 0xFFFF
#define BLACK_COLOR 0x0

void display_init();
void display_draw(uint16_t x, uint16_t y, uint16_t c);
void display_clear(uint16_t c);

#endif // DISPLAY_H
