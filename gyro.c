#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#include "command_buffer.h"
#include "gyro.h"
#include "tetris_uart.h"

// calibration value, offset of the acceleration among the x axis
// set by gyro_calibrate_x_acc(), used by gyro_read_x_acc
static volatile uint16_t x_acc_offset = 0;

// the number of times the gyroscope sampler has to skip
static volatile uint8_t gyro_skip = 0;

// flag marking if the gyro is ready to be sampled
static volatile bool GYRO_READY = false;

// repeating timer structure for the gyro_ready_callback()
static struct repeating_timer gyro_timer;

// inicialize the gyroscope
void gyro_init() {
    i2c_init(i2c_default, GYRO_BAUD_RATE);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
	gyro_reset();
}

// turn on the gyro timer if it isn't running, turn it off otherwise.
void gyro_toggle() {
	if (!cancel_repeating_timer(&gyro_timer)) {
		if (!add_repeating_timer_ms(GYRO_SAMPLE_RATE, gyro_ready_callback,
					NULL, &gyro_timer)) {
			uart_puts(UART_ID, "Couldn't start gyro repeating timer\r\n");
		}
		uart_puts(UART_ID, "GYRO turned on!\r\n");
	} else {
		uart_puts(UART_ID, "GYRO turned off!\r\n");
		GYRO_READY = false;
	}
}

// reset the gyroscope
void gyro_reset() {
	cancel_repeating_timer(&gyro_timer);

	// Two byte reset. First byte register, second byte data.
    uint8_t buf[] = {0x6B, 0x00};
    i2c_write_blocking(i2c_default, GYRO_ADDR, buf, 2, false);
	gyro_calibrate_x_acc();

	// start timer
	if (!add_repeating_timer_ms(GYRO_SAMPLE_RATE, gyro_ready_callback,
				NULL, &gyro_timer)) {
		uart_puts(UART_ID, "Couldn't start gyro repeating timer\r\n");
	}
}

bool gyro_is_ready() {
	return GYRO_READY;
}

// read accelerometer data into the buffer
void gyro_read_acc(int16_t accel[3]) {
    uint8_t buffer[6];

    uint8_t val = 0x3B;
    i2c_write_blocking(i2c_default, GYRO_ADDR, &val, 1, true);
    i2c_read_blocking(i2c_default, GYRO_ADDR, buffer, 6, false);

    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }
}

// read acceleration among the x axis
int16_t gyro_read_x_acc() {
	uint8_t buffer[2];
    uint8_t val = 0x3B;
    i2c_write_blocking(i2c_default, GYRO_ADDR, &val, 1, true);
    i2c_read_blocking(i2c_default, GYRO_ADDR, buffer, 2, false);
	return (buffer[0] << 8 | buffer[1]);
}

// calibrate the acceleration among the x axis
// read some samples, then average them, so when reading the actual values
// the offset can be added to it to make it 0
void gyro_calibrate_x_acc() {
	int32_t accumulator = 0;
	int32_t samples = 100;

	for (int32_t i = 0; i < samples; i++) {
		accumulator += gyro_read_x_acc();
		sleep_ms(5);
	}

	x_acc_offset = - accumulator / samples;
}

// read acceleration data, and write a command to the global command buffer
// if an event was triggered
void gyro_write_to_buffer() {
	GYRO_READY = false;
	int16_t x_acc = gyro_read_x_acc() + x_acc_offset;

	if (x_acc > GYRO_TRESHOLD) {
		command_buffer_write(LEFT_CMD);
		gyro_skip += (GYRO_TIMEOUT / GYRO_SAMPLE_RATE);
	} else if (x_acc < -GYRO_TRESHOLD) {
		command_buffer_write(RIGHT_CMD);
		gyro_skip += (GYRO_TIMEOUT / GYRO_SAMPLE_RATE);
	}
}

// set the GYRO_READY flag to true, with respect to the timeout of the gyro
bool gyro_ready_callback(struct repeating_timer *t) {
	(void)t;
	if (gyro_skip == 0) {
		GYRO_READY = true;
	} else {
		gyro_skip--;
	}
	return true;
}
