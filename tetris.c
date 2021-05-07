#include <stdint.h>

#include "tetris.h"

void tetris_init(struct Tetris *tetris) {
	// empty board
	for (uint8_t i = 0; i < (BOARD_W * BOARD_H) / 8; i++) {
		tetris->board[i] = 0;
	}

	// I tetromino
	// 1 (4x4)
	// 0 0 0 0
	// 1 1 1 1
	// 0 0 0 0
	// 0 0 0
	// 0b1000011110000000 = 0x8780
	tetris->tetrominos[0] = 0x8780;

	// O tetromino
	// 1 (4x4) (it's 2x2 but it rotates the same way)
	// 0 0 0 0
	// 0 1 1 0
	// 0 1 1 0
	// 0 0 0
	// 0b1000001100110000 = 0x8330
	tetris->tetrominos[1] = 0x8330;

	// T tetromino
	// 0 (3x3)
	// 0 0 0
	// 1 1 1
	// 0 1 0
	//
	// 0 0 0 0 0 0
	// 0b0000111010000000 = 0xE80
	tetris->tetrominos[2] = 0xE80;

	// J tetromino
	// 0 (3x3)
	// 0 1 0
	// 0 1 0
	// 1 1 0
	//
	// 0 0 0 0 0 0
	// 0b0010010110000000 = 0x2580
	tetris->tetrominos[3] = 0x2580;

	// L tetromino
	// 0 (3x3)
	// 0 1 0
	// 0 1 0
	// 0 1 1
	//
	// 0 0 0 0 0 0
	// 0b0010010011000000 = 0x24C0
	tetris->tetrominos[4] = 0x24C0;

	// S tetromino
	// 0 (3x3)
	// 0 1 1
	// 1 1 0
	// 0 0 0
	//
	// 0 0 0 0 0 0
	// 0b0011110000000000 = 0x3C00
	tetris->tetrominos[5] = 0x3C00;

	// Z tetromino
	// 0 (3x3)
	// 0 0 0
	// 1 1 0
	// 0 1 1
	//
	// 0 0 0 0 0 0
	// 0b0000110011000000 = 0xCC0
	tetris->tetrominos[6] = 0xCC0;

	tetris_set_new_tetromino(tetris);
	tetris->need_update = false;
	tetris->score = 0;
}


// return the block (bit) in the tetromino with index t at (x, y)
uint8_t tetris_get_tetromino_block(struct Tetris *tetris, uint8_t t,
		uint8_t x, uint8_t y) {
	// get the size from the first bit
	uint8_t size = 3 + (tetris->tetrominos[t] >> 15);
	uint8_t bitindex = y * size + x;

	// handle out of bounds indexes
	return (x >= size || y >= size) ? 0
		: ((tetris->tetrominos[t] >> (14 - bitindex)) & 1);
}

// return the block (bit) on the board at (x, y) without the tetromino on it
uint8_t tetris_get_raw_board_block(struct Tetris *tetris,
		uint8_t x, uint8_t y) {
	uint8_t bitindex = (y * BOARD_W) + x;
	return ((tetris->board[bitindex / 8] >> (7 - (bitindex % 8)))) & 1;
}

// return the block (bit) on the board at (x, y) with the tetromino on it
uint8_t tetris_get_board_block(struct Tetris *tetris, uint8_t x, uint8_t y) {
	uint8_t bitindex = (y * BOARD_W) + x;
	uint8_t tetromino_overlay = tetris_get_tetromino_block(tetris,
			tetris->tetromino, x - tetris->x, y - tetris->y);
	return ((tetris->board[bitindex / 8] >> (7 - (bitindex % 8))) & 1) |
		tetromino_overlay;
}

void tetris_set_board_block(struct Tetris *tetris,
		uint8_t x, uint8_t y, uint8_t val) {
	uint8_t bitindex = (y * BOARD_W) + x;
	if (val) {
		// set bit
		tetris->board[bitindex / 8] |= (1 << (7 - (bitindex % 8)));
	} else {
		// clear bit
		tetris->board[bitindex / 8] &= ~(1 << (7 - (bitindex % 8)));
	}
}

// if the tetromino hangs out of the board, push it in
void tetris_fix_overhang(struct Tetris *tetris) {
	// get the size from the first bit
	uint8_t size = 3 + (tetris->tetrominos[tetris->tetromino] >> 15);

	for (uint8_t i = 0; i < size; i++) {
		for (uint8_t j = 0; j < size; j++) {
			if (tetris_get_tetromino_block(tetris, tetris->tetromino, j, i)) {
				while (j + tetris->x < 0) {
					tetris->x++;
				}
				while (j + tetris->x >= BOARD_W) {
					tetris->x--;
				}
			}
		}
	}
}

void tetris_rotate_tetromino_ccw(struct Tetris *tetris, uint8_t t) {
	// clear tetromino, but keep the size indicator
	uint16_t rotated = tetris->tetrominos[t] & (1 << 15);
	// get the size from the first bit
	uint8_t size = 3 + (tetris->tetrominos[t] >> 15);

	// rotate counter clockwise
	for (uint8_t i = 0; i < size; i++) {
		for (uint8_t j = 0; j < size; j++) {
			rotated |= tetris_get_tetromino_block(tetris, t, size - 1 - i, j)
				<< (14 - i * size - j);
		}
	}

	tetris->tetrominos[t] = rotated;
}

void tetris_rotate_tetromino_cw(struct Tetris *tetris, uint8_t t) {
	// clear tetromino, but keep the size indicator
	uint16_t rotated = tetris->tetrominos[t] & (1 << 15);
	// get the size from the first bit
	uint8_t size = 3 + (tetris->tetrominos[t] >> 15);

	// rotate clockwise
	for (uint8_t i = 0; i < size; i++) {
		for (uint8_t j = 0; j < size; j++) {
			rotated |= tetris_get_tetromino_block(tetris, t, i, size - 1 - j)
				<< (14 - i * size - j);
		}
	}

	tetris->tetrominos[t] = rotated;
}

// return 0 if the current tetromino doesn't collide with blocks on
// the board, returns non zero otherwise
uint8_t tetris_check_overlap(struct Tetris *tetris) {
	// get the size from the first bit
	uint8_t size = 3 + (tetris->tetrominos[tetris->tetromino] >> 15);

	for (uint8_t i = 0; i < size; i++) {
		for (uint8_t j = 0; j < size; j++) {
			if (tetris_get_tetromino_block(tetris, tetris->tetromino, j, i) &&
					tetris_get_raw_board_block(tetris, tetris->x + j,
						tetris->y + i)) {
				return 1;
			}
		}
	}

	return 0;
}

void tetris_rotate_ccw(struct Tetris *tetris) {
	if (tetris->need_update) {
		return;
	}
	tetris_rotate_tetromino_ccw(tetris, tetris->tetromino);
	if (tetris_check_overlap(tetris)) {
		tetris_rotate_tetromino_cw(tetris, tetris->tetromino);
	}
	tetris_fix_overhang(tetris);
	if (tetris_check_collision(tetris)) {
		tetris_save_tetromino_to_board(tetris);
		tetris->need_update = 1;
	}
}

void tetris_rotate_cw(struct Tetris *tetris) {
	if (tetris->need_update) {
		return;
	}
	tetris_rotate_tetromino_cw(tetris, tetris->tetromino);
	if (tetris_check_overlap(tetris)) {
		tetris_rotate_tetromino_ccw(tetris, tetris->tetromino);
	}
	tetris_fix_overhang(tetris);
	if (tetris_check_collision(tetris)) {
		tetris_save_tetromino_to_board(tetris);
		tetris->need_update = 1;
	}
}

void tetris_move_tetromino_right(struct Tetris *tetris) {
	if (tetris->need_update) {
		return;
	}
	tetris->x++;
	if (tetris_check_overlap(tetris)) {
		tetris->x--;
	}
	tetris_fix_overhang(tetris);
	if (tetris_check_collision(tetris)) {
		tetris_save_tetromino_to_board(tetris);
		tetris->need_update = 1;
	}
}

void tetris_move_tetromino_left(struct Tetris *tetris) {
	if (tetris->need_update) {
		return;
	}
	tetris->x--;
	if (tetris_check_overlap(tetris)) {
		tetris->x++;
	}
	tetris_fix_overhang(tetris);
	if (tetris_check_collision(tetris)) {
		tetris_save_tetromino_to_board(tetris);
		tetris->need_update = 1;
	}
}

void tetris_set_new_tetromino(struct Tetris *tetris) {
	// set a new random tetromino
	tetris->tetromino = tetris_random() % N_TETROMINOS;

	// get the size from the first bit
	uint8_t size = 3 + (tetris->tetrominos[tetris->tetromino] >> 15);

	// set a new random starting position
	tetris->x = tetris_random() % (BOARD_W - size);
	tetris->y = -(int8_t)size;

	// rotate the tetromino to a random position
	uint8_t rotations = tetris_random() % 4;
	if (rotations == 3) {
		tetris_rotate_ccw(tetris);
	} else {
		while (rotations--) {
			tetris_rotate_cw(tetris);
		}
	}
}

uint8_t tetris_check_collision(struct Tetris *tetris) {
	// get the size from the first bit
	uint8_t size = 3 + (tetris->tetrominos[tetris->tetromino] >> 15);
	int8_t x;
	int8_t y;

	for (uint8_t i = 0; i < size; i++) {
		for (uint8_t j = 0; j < size; j++) {
			// if there is a block
			if (tetris_get_tetromino_block(tetris, tetris->tetromino, j, i)) {
				x = tetris->x + j;
				y = tetris->y + i;

				// check if the block is at the bottom
				if (y == BOARD_H - 1) {
					return 1;
				} else if (x >= 0 && x < BOARD_W && y >= 0 && y < BOARD_H - 1) {
					// check if there is a block under it
					if (tetris_get_raw_board_block(tetris, x, y + 1)) {
						return 1;
					}
				}
			}
		}
	}

	return 0;
}

void tetris_save_tetromino_to_board(struct Tetris *tetris) {
	// get the size from the first bit
	uint8_t size = 3 + (tetris->tetrominos[tetris->tetromino] >> 15);

	for (uint8_t i = 0; i < size; i++) {
		for (uint8_t j = 0; j < size; j++) {
			// if there is a block
			if (tetris_get_tetromino_block(tetris, tetris->tetromino, j, i)) {
				tetris_set_board_block(tetris, tetris->x + j, tetris->y + i, 1);
			}
		}
	}
}

void tetris_remove_row(struct Tetris *tetris, uint8_t r) {
	// shift all rows down
	for (uint8_t i = r; i > 0; i--) {
		for (uint8_t j = 0; j < BOARD_W; j++) {
			tetris_set_board_block(tetris, j, i,
					tetris_get_raw_board_block(tetris, j, i - 1));
		}
	}

	// set first row to 0
	for (uint8_t j = 0; j < BOARD_W; j++) {
		tetris_set_board_block(tetris, j, 0, 0);
	}
}

// remove the full rows from the board
void tetris_remove_full_rows(struct Tetris *tetris) {
	uint8_t is_full;
	for (int8_t i = BOARD_H - 1; i >= 0; i--) {
		is_full = 1;
		for (uint8_t j = 0; is_full && j < BOARD_W; j++) {
			is_full = tetris_get_board_block(tetris, j, i);
		}
		if (is_full) {
			// increase score for each removed row
			tetris->score++;
			tetris_remove_row(tetris, i);
			i++;
		}
	}
}

// returns non 0 if game is over
uint8_t tetris_is_game_over(struct Tetris *tetris) {
	// get the size from the first bit
	int8_t size = 3 + (tetris->tetrominos[tetris->tetromino] >> 15);

	for (int8_t i = tetris->y; i < 0; i++) {
		for (uint8_t j = 0; j < size; j++) {
			// check if there is a block outside the board
			if (tetris_get_tetromino_block(tetris, tetris->tetromino, j, i + size)) {
				return 1;
			}
		}
	}

	return 0;
}

// returns non 0 if the game is over
uint8_t tetris_update(struct Tetris *tetris) {
	if (tetris_check_collision(tetris) || tetris->need_update) {
		tetris->need_update = 0;
		tetris_save_tetromino_to_board(tetris);
		tetris_remove_full_rows(tetris);

		if (tetris_is_game_over(tetris)) {
			return 1;
		}

		tetris_set_new_tetromino(tetris);
	} else {
		tetris->y++;
	}

	return 0;
}

void tetris_drop(struct Tetris *tetris) {
    if (tetris->need_update) {
        return;
    } else {
        tetris->need_update = 1;
    }
    
    while (!tetris_check_collision(tetris)) {
        tetris->y++;
    }
}
