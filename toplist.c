#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include "toplist.h"

// where the toplist starts
#define FLASH_TARGET_OFFSET (FLASH_PAGE_SIZE * 1024)

// pointer to the content of the flash
static const uint8_t *flash_content = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

// write the new scores to the flash
static void toplist_write_scores(uint16_t *scores) {
	uint8_t buffer[FLASH_PAGE_SIZE] = {0};
	for (uint8_t i = 0; i < TOPLIST_LENGTH; i++) {
		buffer[2 * i] = scores[i] >> 8;
		buffer[2 * i + 1] = scores[i];
	}

	// interrupts need to be disabled during writing the flash
	uint32_t ints = save_and_disable_interrupts();
	flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
	restore_interrupts(ints);
}

// set the toplist scores to 0
void toplist_clear() {
	uint8_t buffer[FLASH_PAGE_SIZE] = {0};
	uint32_t ints = save_and_disable_interrupts();

	// interrupts need to be disabled during writing the flash
	flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
	restore_interrupts(ints);
}

// add a new score to the toplist if it qualifies
void toplist_add(uint16_t score) {
	uint16_t scores[TOPLIST_LENGTH];
	uint8_t higher_then = TOPLIST_LENGTH;

	// read current
	for (uint8_t i = 0; i < TOPLIST_LENGTH; i++) {
		scores[i] = (flash_content[i * 2] << 8) | flash_content[i * 2 + 1];
		if (score > scores[i] && higher_then == TOPLIST_LENGTH) {
			higher_then = i;
		}
	}

	if (higher_then < TOPLIST_LENGTH) {
		// shift scores
		for (uint8_t i = TOPLIST_LENGTH - 1; i > higher_then; i--) {
			scores[i] = scores[i - 1];
		}

		// insert score
		scores[higher_then] = score;

		// write to flash
		toplist_write_scores(scores);
	}
}

// return the nth score in the toplist
uint16_t toplist_read(uint8_t nth) {
	if (nth >= TOPLIST_LENGTH) {
		return -1;
	}
	return (flash_content[nth * 2] << 8) | flash_content[nth * 2 + 1];
}
