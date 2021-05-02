#ifndef GYRO_H
#define GYRO_H

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"

#define GYRO_ADDR 0x68
#define GYRO_BAUD_RATE 400000
#define GYRO_SAMPLE_RATE 20
#define GYRO_TIMEOUT 140
#define GYRO_TRESHOLD 1700

void gyro_init();
void gyro_reset();
bool gyro_is_ready();
void gyro_read_acc(int16_t accel[3]);
int16_t gyro_read_x_acc();
void gyro_calibrate_x_acc();
void gyro_write_to_buffer();
bool gyro_ready_callback(struct repeating_timer *t);

#endif // GYRO_H
