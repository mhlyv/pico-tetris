#include <stdint.h>

#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "display.h"
#include "tetris.h"

enum DC {
	DATA = 1,
	CMD = 0 
};

// commands for ST7735S
// source: https://www.crystalfontz.com/controllers/Sitronix/ST7735S/320
enum COMMAND {
	NOP       = 0x00,
	SWRESET   = 0x01, // Software Reset 
	RDDID     = 0x04, // Read Display ID 
	RDDST     = 0x09, // Read Display Status 
	RDDPM     = 0x0A, // Read Display Power Mode 
	RDDMADCTL = 0x0B, // Read Display MADCTL 
	RDDCOLMOD = 0x0C, // Read Display Pixel Format 
	RDDIM     = 0x0D, // Read Display Image Mode 
	RDDSM     = 0x0E, // Read Display Signal Mode 
	RDDSDR    = 0x0F, // Read Display Self-Diagnostic Result 
	SLPIN     = 0x10, // Sleep In 
	SLPOUT    = 0x11, // Sleep Out 
	PTLON     = 0x12, // Partial Display Mode On 
	NORON     = 0x13, // Normal Display Mode On 
	INVOFF    = 0x20, // Display Inversion Off 
	INVON     = 0x21, // Display Inversion On 
	GAMSET    = 0x26, // Gamma Set 
	DISPOFF   = 0x28, // Display Off 
	DISPON    = 0x29, // Display On 
	CASET     = 0x2A, // Column Address Set 
	RASET     = 0x2B, // Row Address Set 
	RAMWR     = 0x2C, // Memory Write 
	RGBSET    = 0x2D, // Color Setting 4k, 65k, 262k 
	RAMRD     = 0x2E, // Memory Read 
	PTLAR     = 0x30, // Partial Area 
	SCRLAR    = 0x33, // Scroll Area Set 
	TEOFF     = 0x34, // Tearing Effect Line OFF 
	TEON      = 0x35, // Tearing Effect Line ON 
	MADCTL    = 0x36, // Memory Data Access Control 
	VSCSAD    = 0x37, // Vertical Scroll Start Address of RAM 
	IDMOFF    = 0x38, // Idle Mode Off 
	IDMON     = 0x39, // Idle Mode On 
	COLMOD    = 0x3A, // Interface Pixel Format 
	RDID1     = 0xDA, // Read ID1 Value 
	RDID2     = 0xDB, // Read ID2 Value 
	RDID3     = 0xDC, // Read ID3 Value 
	FRMCTR1   = 0xB1, // Frame Rate Control in normal mode, full colors 
	FRMCTR2   = 0xB2, // Frame Rate Control in idle mode, 8 colors 
	FRMCTR3   = 0xB3, // Frame Rate Control in partial mode, full colors 
	INVCTR    = 0xB4, // Display Inversion Control 
	PWCTR1    = 0xC0, // Power Control 1 
	PWCTR2    = 0xC1, // Power Control 2 
	PWCTR3    = 0xC2, // Power Control 3 in normal mode, full colors 
	PWCTR4    = 0xC3, // Power Control 4 in idle mode 8colors 
	PWCTR5    = 0xC4, // Power Control 5 in partial mode, full colors 
	VMCTR1    = 0xC5, // VCOM Control 1 
	VMOFCTR   = 0xC7, // VCOM Offset Control 
	WRID2     = 0xD1, // Write ID2 Value 
	WRID3     = 0xD2, // Write ID3 Value 
	NVFCTR1   = 0xD9, // NVM Control Status 
	NVFCTR2   = 0xDE, // NVM Read Command 
	NVFCTR3   = 0xDF, // NVM Write Command 
	GMCTRP1   = 0xE0, // Gamma +Polarity Correction Characteristics Setting 
	GMCTRN1   = 0xE1, // Gamma -Polarity Correction Characteristics Setting 
	GCV       = 0xFC, // Gate Pump Clock Frequency Variable 
};

void display_wr_bus(uint8_t data) {
	gpio_put(DISPLAY_CS_PIN, 0);

	uint8_t *buff = (uint8_t *)&data;
	spi_write_blocking(DISPLAY_SPI, buff, 1);

	gpio_put(DISPLAY_CS_PIN, 1);
}

void display_wr_8(uint8_t data) {
	gpio_put(DISPLAY_DC_PIN, 1);
	display_wr_bus(data);
}

void display_wr_16(uint16_t data) {
	gpio_put(DISPLAY_DC_PIN, 1);
	display_wr_bus(data >> 8);
	display_wr_bus(data);
}

void display_wr_cmd(uint8_t cmd) {
	gpio_put(DISPLAY_DC_PIN, 0);
	display_wr_bus(cmd);
}

void display_set_address(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2) {
	display_wr_cmd(CASET);
	display_wr_16(x1 + 26);
	display_wr_16(x2 + 26);
	display_wr_cmd(RASET);
	display_wr_16(y1 + 1);
	display_wr_16(y2 + 1);
	display_wr_cmd(RAMWR);
}

void spi_config() {
	gpio_init(DISPLAY_SCK_PIN);
	gpio_set_function(DISPLAY_SCK_PIN, GPIO_FUNC_SPI);

	gpio_init(DISPLAY_TX_PIN);
	gpio_set_function(DISPLAY_TX_PIN, GPIO_FUNC_SPI);

	gpio_init(DISPLAY_CS_PIN);
	gpio_set_dir(DISPLAY_CS_PIN, GPIO_OUT);

	spi_init(DISPLAY_SPI, DISPLAY_SPI_FREQ);

	spi_set_format(DISPLAY_SPI,
		8, // 8 data_bits
		SPI_CPOL_0,
		SPI_CPHA_0,
		SPI_MSB_FIRST
	);

	gpio_put(DISPLAY_CS_PIN, 1);
}

void display_init() {
	gpio_init(DISPLAY_DC_PIN);
	gpio_set_dir(DISPLAY_DC_PIN, GPIO_OUT);

	gpio_init(DISPLAY_RST_PIN);
	gpio_set_dir(DISPLAY_RST_PIN, GPIO_OUT);

	spi_config();

	gpio_put(DISPLAY_RST_PIN, 0);
	sleep_ms(200);
	gpio_put(DISPLAY_RST_PIN, 1);
	sleep_ms(20);

	display_wr_cmd(SLPOUT);
	sleep_ms(100);

	display_wr_cmd(INVON);

	display_wr_cmd(FRMCTR1);
	display_wr_8(0x05);
	display_wr_8(0x3A);
	display_wr_8(0x3A);

	display_wr_cmd(FRMCTR2);
	display_wr_8(0x05);
	display_wr_8(0x3A);
	display_wr_8(0x3A);

	display_wr_cmd(FRMCTR3);
	display_wr_8(0x05);  
	display_wr_8(0x3A);
	display_wr_8(0x3A);
	display_wr_8(0x05);
	display_wr_8(0x3A);
	display_wr_8(0x3A);

	display_wr_cmd(INVCTR);
	display_wr_8(0x03);

	display_wr_cmd(PWCTR1);
	display_wr_8(0x62);
	display_wr_8(0x02);
	display_wr_8(0x04);

	display_wr_cmd(PWCTR2);
	display_wr_8(0xC0);

	display_wr_cmd(PWCTR3);
	display_wr_8(0x0D);
	display_wr_8(0x00);

	display_wr_cmd(PWCTR4);
	display_wr_8(0x8D);
	display_wr_8(0x6A);   

	display_wr_cmd(PWCTR5);
	display_wr_8(0x8D); 
	display_wr_8(0xEE); 

	display_wr_cmd(VMCTR1);
	display_wr_8(0x0E);    

	display_wr_cmd(GMCTRP1);
	display_wr_8(0x10);
	display_wr_8(0x0E);
	display_wr_8(0x02);
	display_wr_8(0x03);
	display_wr_8(0x0E);
	display_wr_8(0x07);
	display_wr_8(0x02);
	display_wr_8(0x07);
	display_wr_8(0x0A);
	display_wr_8(0x12);
	display_wr_8(0x27);
	display_wr_8(0x37);
	display_wr_8(0x00);
	display_wr_8(0x0D);
	display_wr_8(0x0E);
	display_wr_8(0x10);

	display_wr_cmd(GMCTRN1);
	display_wr_8(0x10);
	display_wr_8(0x0E);
	display_wr_8(0x03);
	display_wr_8(0x03);
	display_wr_8(0x0F);
	display_wr_8(0x06);
	display_wr_8(0x02);
	display_wr_8(0x08);
	display_wr_8(0x0A);
	display_wr_8(0x13);
	display_wr_8(0x26);
	display_wr_8(0x36);
	display_wr_8(0x00);
	display_wr_8(0x0D);
	display_wr_8(0x0E);
	display_wr_8(0x10);

	display_wr_cmd(COLMOD);
	display_wr_8(0x05);	// 16 bit colors

	display_wr_cmd(MADCTL);
	display_wr_8(0x00 | (1 << 3));

	display_wr_cmd(DISPON);
}

void display_clear(uint16_t color) {
	display_set_address(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
	for(uint8_t i = 0; i < DISPLAY_HEIGHT; i++) {
		for (uint8_t j = 0; j < DISPLAY_WIDTH; j++) {
			display_wr_16(color);
		}
	}
}

void display_draw_block(uint8_t x, uint8_t y, uint16_t color) {
	display_set_address(
		x * DISPLAY_BLOCK_SIZE,
		y * DISPLAY_BLOCK_SIZE,
		(x + 1) * DISPLAY_BLOCK_SIZE - 1,
		(y + 1) * DISPLAY_BLOCK_SIZE - 1
	);
	for (uint8_t i = 0; i < DISPLAY_BLOCK_SIZE * DISPLAY_BLOCK_SIZE; i++) {
		display_wr_16(color);
	}
}

void display_tetris(struct Tetris *tetris) {
	for (uint8_t i = 0; i < BOARD_H; i++) {
		for (uint8_t j = 0; j < BOARD_W; j++) {
			display_draw_block(j, i,
				tetris_get_board_block(tetris, j, i) ?
				WHITE_COLOR : BLACK_COLOR);
		}
	}
}
