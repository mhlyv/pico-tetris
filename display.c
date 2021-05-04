#include <stdint.h>

#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "display.h"

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

static void display_set_dc(enum DC dc) {
	gpio_put(DISPLAY_DC_PIN, dc);
}

static inline void display_wr_bus(uint8_t data) {
	gpio_put(DISPLAY_CS_PIN, 0);
	spi_write_blocking(DISPLAY_SPI, &data, 1);
	gpio_put(DISPLAY_CS_PIN, 1);
}

// write 8 bit data
static inline void display_wr_8(uint8_t data) {
	display_set_dc(DATA);
	display_wr_bus(data);
}

// write 16 bit data
static inline void display_wr_16(uint16_t data) {
	display_set_dc(DATA);
	display_wr_bus(data >> 8);
	display_wr_bus(data);
}

// write 8 bit command
static inline void display_wr_cmd(uint8_t cmd) {
	display_set_dc(CMD);
	display_wr_bus(cmd);
}

// configure display
static inline void display_configure() {
	display_wr_cmd(SWRESET); // software reset
	display_wr_cmd(SLPOUT); // no sleep
	display_wr_cmd(DISPOFF); // display off

	display_wr_cmd(FRMCTR1); // frame frequency in normal mode
	display_wr_8(0x00); // 1. parameter (RTNA)
	display_wr_8(0x3F); // 2. paramter (FPA)
	display_wr_8(0x3F); // 3. paramter (BPA)

	display_wr_cmd(FRMCTR2); // frame frequency in idle mode
	display_wr_8(0x0F); // 1. parameter (RTNB)
	display_wr_8(0x01); // 2. paramter (FPB)
	display_wr_8(0x01); // 3. paramter (BPB)

	display_wr_cmd(FRMCTR3); // frame frequency in partial mode
	display_wr_8(0x05); // 1. parameter (RTNC)
	display_wr_8(0x3C); // 2. parameter (FPC)
	display_wr_8(0x3C); // 3. parameter (BPC)
	display_wr_8(0x05); // 4. parameter (RTND)
	display_wr_8(0x3C); // 5. parameter (FPD)
	display_wr_8(0x3C); // 6. parameter (BPD)

	display_wr_cmd(INVCTR); // display inversion control
	display_wr_8(0x03); // 1. parameter (NLA, NLP, NLC)

	display_wr_cmd(PWCTR1); // power control 1
	display_wr_8(0xFC); // 1. parameter (AVDD[0:2], VRHP[0:4])
	display_wr_8(0x08); // 2. parameter (VRHN[0:4])
	display_wr_8(0x06); // 3. parameter (MODE[0:2], 0001, VRHN[5], VRHP[5])

	display_wr_cmd(PWCTR2); // power control 2
	display_wr_8(0xC0); // 1. parameter (VGH25, ??, VGSEL, VGHBT)

	display_wr_cmd(PWCTR3); // power control 3
	display_wr_8(0x0D); // 1. parameter (DCA[8:9], SAPA, APA)
	display_wr_8(0x00); // 2. parameter (DCA[0:7])

	display_wr_cmd(PWCTR4); // power control 4 (idle mode/8 colors)
	display_wr_8(0x8D); // 1. parameter (DCB[8:9], SAPB, APB)
	display_wr_8(0x2A); // 2. parameter (DCB[0:7])

	display_wr_cmd(PWCTR5); // power control 5 (partial mode/full colors)
	display_wr_8(0x8D); // 1. parameter (DCC[8:9], SAPC, APC)
	display_wr_8(0xEE); // 2. parameter (DCC[0:7])

	display_wr_cmd(GCV); // gate pump clock frequency
	display_wr_8(0xD8); // 1. parameter (auto freq, max power save)

	display_wr_cmd(NVFCTR1); // NVM control status
	display_wr_8(0x40); // 1. parameter, flip VMF_EN required by VMOFCTR

	display_wr_cmd(VMCTR1); // VCOM voltage setting
	display_wr_8(0x0F); // 1. parameter (VCOMS)

	display_wr_cmd(VMOFCTR); // VCOM offset control
	display_wr_8(0x10); // 1. parameter (VMF)

	display_wr_cmd(GAMSET); // Gamma set curve
	display_wr_8(0x08); // 1. parameter (GC)

	display_wr_cmd(MADCTL); // Memory Data Access Control
	display_wr_8(0x60); // 1. parameter (MY, MX, MV, ML, RGB, MH)

	display_wr_cmd(COLMOD); // interface pixel format
	display_wr_8(0x05); // 1. parameter (IFPF) set 16 bit colors

	display_wr_cmd(GMCTRP1); // Gamma +polarity correction
	display_wr_8(0x02); // 1. parameter
	display_wr_8(0x1c); // 2. parameter
	display_wr_8(0x07); // 3. parameter
	display_wr_8(0x12); // 4. parameter
	display_wr_8(0x37); // 5. parameter
	display_wr_8(0x32); // 6. parameter
	display_wr_8(0x29); // 7. parameter
	display_wr_8(0x2c); // 8. parameter
	display_wr_8(0x29); // 9. parameter
	display_wr_8(0x25); // 10. parameter
	display_wr_8(0x2b); // 11. parameter
	display_wr_8(0x39); // 12. parameter
	display_wr_8(0x00); // 13. parameter
	display_wr_8(0x01); // 14. parameter
	display_wr_8(0x03); // 15. parameter
	display_wr_8(0x10); // 16. parameter

	display_wr_cmd(GMCTRN1); // Gamma -polarity correction
	display_wr_8(0x03); // 1. parameter
	display_wr_8(0x1d); // 2. parameter
	display_wr_8(0x07); // 3. parameter
	display_wr_8(0x06); // 4. parameter
	display_wr_8(0x2E); // 5. parameter
	display_wr_8(0x2C); // 6. parameter
	display_wr_8(0x29); // 7. parameter
	display_wr_8(0x2c); // 8. parameter
	display_wr_8(0x2e); // 9. parameter
	display_wr_8(0x2e); // 10. parameter
	display_wr_8(0x37); // 11. parameter
	display_wr_8(0x3f); // 12. parameter
	display_wr_8(0x00); // 13. parameter
	display_wr_8(0x00); // 14. parameter
	display_wr_8(0x02); // 15. parameter
	display_wr_8(0x10); // 16. parameter

	display_wr_cmd(CASET); // set column address
	display_wr_8(0x00); // 1. parameter (XS[8:15])
	display_wr_8(0x00); // 2. parameter (XS[0:7])
	display_wr_8(0x00); // 3. parameter (XE[8:15])
	display_wr_8(DISPLAY_HEIGHT - 1); // 4. parameter (XE[0:7])

	display_wr_cmd(RASET); // set row address
	display_wr_8(0x00); // 1. parameter (YS[8:15])
	display_wr_8(0x00); // 2. parameter (YS[0:7])
	display_wr_8(0x00); // 3. parameter (YE[8:15])
	display_wr_8(DISPLAY_WIDTH - 1); // 4. parameter (YE[0:7])

	display_wr_cmd(INVON); // display inversion on
	display_wr_cmd(IDMOFF); // idle mode off
	display_wr_cmd(NORON); // normal mode on
	display_wr_cmd(DISPON); // display on
}

void display_init() {
	// set up pins
	gpio_init(DISPLAY_SCK_PIN);
	gpio_set_function(DISPLAY_SCK_PIN, GPIO_FUNC_SPI);

	gpio_init(DISPLAY_TX_PIN);
	gpio_set_function(DISPLAY_TX_PIN, GPIO_FUNC_SPI);

	gpio_init(DISPLAY_CS_PIN);
	gpio_set_function(DISPLAY_CS_PIN, GPIO_OUT);

	gpio_init(DISPLAY_DC_PIN);
	gpio_set_function(DISPLAY_DC_PIN, GPIO_OUT);

	gpio_init(DISPLAY_RST_PIN);
	gpio_set_function(DISPLAY_RST_PIN, GPIO_OUT);

	spi_init(DISPLAY_SPI, DISPLAY_SPI_FREQ);
	spi_set_format(
			DISPLAY_SPI, 
			8, // 8 data
			SPI_CPOL_0,
			SPI_CPHA_0,
			SPI_MSB_FIRST
	);

	// hard reset
	gpio_put(DISPLAY_RST_PIN, 1);
	gpio_put(DISPLAY_CS_PIN, 0);
	gpio_put(DISPLAY_DC_PIN, 0);
	sleep_ms(2);
	gpio_put(DISPLAY_RST_PIN, 0);
	sleep_ms(2);
	gpio_put(DISPLAY_RST_PIN, 1);
	sleep_ms(2);
	gpio_put(DISPLAY_CS_PIN, 1);

	display_configure();
}

static inline void display_set_address(
		uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
	display_wr_cmd(CASET); // column address set
	display_wr_8(0x00);
	display_wr_8(x0);
	display_wr_8(0x00);
	display_wr_8(x1);

	display_wr_cmd(RASET); // column address set
	display_wr_8(0x00);
	display_wr_8(y0);
	display_wr_8(0x00);
	display_wr_8(y1);

	display_wr_cmd(RAMWR);
}

void display_clear(uint16_t color) {
	display_set_address(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
	for (uint8_t i = 0; i < DISPLAY_HEIGHT; i++) {
		for (uint8_t j = 0; j < DISPLAY_WIDTH; j++) {
			display_wr_16(color);
		}
	}
}
