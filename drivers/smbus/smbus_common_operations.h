/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_SMBUS_SMBUS_COMMON_OPERATIONS_H_
#define ZEPHYR_DRIVERS_SMBUS_SMBUS_COMMON_OPERATIONS_H_

#include <stdint.h>
#include <zephyr/drivers/smbus.h>

struct device;

/* Common SMBus protocol operations for use by any backend driver */

/*
 * Drivers using these helpers must place this struct as the first member of
 * their dev->data type so common ops can recover the SMBus config flags
 * (needed for PEC).
 */
struct smbus_common_data {
	uint32_t config;
};

/*
 * Drivers using these helpers must place this struct as the first member of
 * their dev->config type so common ops can recover the backing I2C device.
 */
struct smbus_common_i2c_config {
	const struct device *i2c_dev;
};

int smbus_common_quick(const struct device *dev, uint16_t periph_addr, enum smbus_direction rw);

int smbus_common_byte_write(const struct device *dev, uint16_t periph_addr, uint8_t data_byte);

int smbus_common_byte_read(const struct device *dev, uint16_t periph_addr, uint8_t *data_byte);

int smbus_common_byte_data_write(const struct device *dev, uint16_t periph_addr,
				 uint8_t command, uint8_t data_byte);

int smbus_common_byte_data_read(const struct device *dev, uint16_t periph_addr, uint8_t command,
				uint8_t *data_byte);

int smbus_common_word_data_write(const struct device *dev, uint16_t periph_addr, uint8_t command,
				 uint16_t data_word);

int smbus_common_word_data_read(const struct device *dev, uint16_t periph_addr, uint8_t command,
				uint16_t *data_word);

int smbus_common_pcall(const struct device *dev, uint16_t periph_addr, uint8_t command,
		       uint16_t send_word, uint16_t *recv_word);

int smbus_common_block_write(const struct device *dev, uint16_t periph_addr, uint8_t command,
			     uint8_t count, uint8_t *buf);

int smbus_common_block_read(const struct device *dev, uint16_t periph_addr, uint8_t command,
			    uint8_t *count, uint8_t *buf);

int smbus_common_block_pcall(const struct device *dev, uint16_t addr, uint8_t cmd,
			     uint8_t send_count, uint8_t *send_buf, uint8_t *recv_count,
			     uint8_t *recv_buf);

int smbus_common_cancel(const struct device *dev);

int smbus_common_uncancel(const struct device *dev);

#endif /* ZEPHYR_DRIVERS_SMBUS_SMBUS_COMMON_OPERATIONS_H_ */
