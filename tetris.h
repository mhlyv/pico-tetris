#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>
#include "pico/stdlib.h"

#define BOARD_W 10
#define BOARD_H 20
#define N_TETROMINOS 7

struct Tetris {
	// Tetris board represented as 200 bits.
	uint8_t board[(BOARD_W * BOARD_H) / 8];

	// Tetrominos represented as bits.
	// Order: I, O, T, J, L, S, Z.
	// The last bit is always 0, so instead the first bit represents
	// if the given tetromino is 3x3 (0) or 4x4 (1).
	uint16_t tetrominos[N_TETROMINOS];

	// The index of the currently active tetromino [0 .. 6].
	uint8_t tetromino;

	// Coordinates of the current active tetromino (top left corner).
	int8_t x;
	int8_t y;
	uint8_t need_update;
};

// this is just a forward declaration, the user must implement this
uint8_t tetris_random();
void tetris_init(struct Tetris *tetris);
uint8_t tetris_get_tetromino_block(struct Tetris *tetris, uint8_t t, uint8_t x, uint8_t y);
uint8_t tetris_get_raw_board_block(struct Tetris *tetris, uint8_t x, uint8_t y);
uint8_t tetris_get_board_block(struct Tetris *tetris, uint8_t x, uint8_t y);
void tetris_set_board_block(struct Tetris *tetris, uint8_t x, uint8_t y, uint8_t val);
void tetris_fix_overhang(struct Tetris *tetris);
void tetris_rotate_tetromino_ccw(struct Tetris *tetris, uint8_t t);
void tetris_rotate_tetromino_cw(struct Tetris *tetris, uint8_t t);
uint8_t tetris_check_overlap(struct Tetris *tetris);
void tetris_rotate_ccw(struct Tetris *tetris);
void tetris_rotate_cw(struct Tetris *tetris);
void tetris_move_tetromino_right(struct Tetris *tetris);
void tetris_move_tetromino_left(struct Tetris *tetris);
void tetris_set_new_tetromino(struct Tetris *tetris);
uint8_t tetris_check_collision(struct Tetris *tetris);
void tetris_save_tetromino_to_board(struct Tetris *tetris);
void tetris_remove_row(struct Tetris *tetris, uint8_t r);
void tetris_remove_full_rows(struct Tetris *tetris);
void tetris_update(struct Tetris *tetris);

#endif // TETRIS_H
