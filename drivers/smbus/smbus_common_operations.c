/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/util.h>

#include "smbus_common_operations.h"
#include "smbus_utils.h"

static const struct device *smbus_common_i2c_dev(const struct device *dev)
{
	const struct smbus_common_i2c_config *cfg = dev->config;

	return cfg->i2c_dev;
}

static uint32_t smbus_common_flags(const struct device *dev)
{
	const struct smbus_common_data *data = dev->data;

	return data->config;
}

int smbus_common_quick(const struct device *dev, uint16_t periph_addr, enum smbus_direction rw)
{
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);

	switch (rw) {
	case SMBUS_MSG_WRITE:
		return i2c_write(i2c_dev, NULL, 0, periph_addr);
	case SMBUS_MSG_READ:
		return i2c_read(i2c_dev, NULL, 0, periph_addr);
	default:
		return -EINVAL;
	}
}

int smbus_common_byte_write(const struct device *dev, uint16_t periph_addr, uint8_t data_byte)
{
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = &data_byte,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = &pec,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
	};

	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	smbus_write_prepare_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
	return i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);
}

int smbus_common_byte_read(const struct device *dev, uint16_t periph_addr, uint8_t *data_byte)
{
	int ret;
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t pec = 0;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = data_byte,
			.len = 1,
			.flags = I2C_MSG_READ,
		},
		{
			.buf = &pec,
			.len = 1,
			.flags = I2C_MSG_READ,
		},
	};

	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	ret = i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);
	if (ret < 0) {
		return ret;
	}

	return smbus_read_check_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
}

int smbus_common_byte_data_write(const struct device *dev, uint16_t periph_addr,
				 uint8_t command, uint8_t data_byte)
{
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = &command,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = &data_byte,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = &pec,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
	};

	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	smbus_write_prepare_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
	return i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);
}

int smbus_common_byte_data_read(const struct device *dev, uint16_t periph_addr, uint8_t command,
				uint8_t *data_byte)
{
	int ret;
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = (uint8_t *)&command,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = data_byte,
			.len = 1,
			.flags = I2C_MSG_READ | I2C_MSG_RESTART,
		},
		{
			.buf = &pec,
			.len = 1,
			.flags = I2C_MSG_READ,
		},
	};

	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	ret = i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);
	if (ret < 0) {
		return ret;
	}

	return smbus_read_check_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
}

int smbus_common_word_data_write(const struct device *dev, uint16_t periph_addr, uint8_t command,
				 uint16_t data_word)
{
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = &command,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = (uint8_t *)&data_word,
			.len = 2,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = &pec,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
	};

	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	smbus_write_prepare_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
	return i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);
}

int smbus_common_word_data_read(const struct device *dev, uint16_t periph_addr, uint8_t command,
				uint16_t *data_word)
{
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t buf[2];
	uint8_t pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = (uint8_t *)&command,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = buf,
			.len = 2,
			.flags = I2C_MSG_READ | I2C_MSG_RESTART,
		},
		{
			.buf = &pec,
			.len = 1,
			.flags = I2C_MSG_READ,
		},
	};

	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	int ret = i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);

	if (ret == 0) {
		*data_word = buf[0] | (buf[1] << 8);
		ret = smbus_read_check_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
	}

	return ret;
}

int smbus_common_pcall(const struct device *dev, uint16_t periph_addr, uint8_t command,
		       uint16_t send_word, uint16_t *recv_word)
{
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = &command,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = (uint8_t *)&send_word,
			.len = 2,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = (uint8_t *)recv_word,
			.len = 2,
			.flags = I2C_MSG_READ | I2C_MSG_RESTART,
		},
		{
			.buf = &pec,
			.len = 1,
			.flags = I2C_MSG_READ,
		},
	};

	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	int ret = i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);

	if (ret < 0) {
		return ret;
	}

	return smbus_read_check_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
}

int smbus_common_block_write(const struct device *dev, uint16_t periph_addr, uint8_t command,
			     uint8_t count, uint8_t *buf)
{
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = &command,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = &count,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = buf,
			.len = count,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = &pec,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
	};

	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	smbus_write_prepare_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
	return i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);
}

int smbus_common_block_read(const struct device *dev, uint16_t periph_addr, uint8_t command,
			    uint8_t *count, uint8_t *buf)
{
	int ret;
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t received_pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = (uint8_t *)&command,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = NULL, /* points to msgs[2].len, set below */
			.len = 1,
			.flags = I2C_MSG_READ | I2C_MSG_RESTART,
		},
		{
			.buf = buf,
			.len = 0, /* written by previous message */
			.flags = I2C_MSG_READ,
		},
		{
			.buf = &received_pec,
			.len = 1,
			.flags = I2C_MSG_READ,
		},
	};

	msgs[1].buf = (uint8_t *)&msgs[2].len;
	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	ret = i2c_transfer(i2c_dev, msgs, num_msgs, periph_addr);
	if (ret < 0) {
		return ret;
	}

	*count = msgs[2].len;
	return smbus_read_check_pec(smbus_common_flags(dev), periph_addr, msgs, num_msgs);
}

int smbus_common_block_pcall(const struct device *dev, uint16_t addr, uint8_t cmd,
			     uint8_t send_count, uint8_t *send_buf, uint8_t *recv_count,
			     uint8_t *recv_buf)
{
	int ret;
	const struct device *i2c_dev = smbus_common_i2c_dev(dev);
	uint8_t received_pec;
	uint8_t num_msgs;
	struct i2c_msg msgs[] = {
		{
			.buf = &cmd,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = &send_count,
			.len = 1,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = send_buf,
			.len = send_count,
			.flags = I2C_MSG_WRITE,
		},
		{
			.buf = NULL, /* points to msgs[4].len, set below */
			.len = 1,
			.flags = I2C_MSG_READ | I2C_MSG_RESTART,
		},
		{
			.buf = recv_buf,
			.len = 0, /* written by previous message */
			.flags = I2C_MSG_READ,
		},
		{
			.buf = &received_pec,
			.len = 1,
			.flags = I2C_MSG_READ,
		},
	};

	msgs[3].buf = (uint8_t *)&msgs[4].len;
	num_msgs = smbus_pec_num_msgs(smbus_common_flags(dev), ARRAY_SIZE(msgs));
	ret = i2c_transfer(i2c_dev, msgs, num_msgs, addr);
	if (ret < 0) {
		return ret;
	}

	*recv_count = msgs[4].len;
	return smbus_read_check_pec(smbus_common_flags(dev), addr, msgs, num_msgs);
}

int smbus_common_cancel(const struct device *dev)
{
	return i2c_cancel(smbus_common_i2c_dev(dev));
}

int smbus_common_uncancel(const struct device *dev)
{
	return i2c_uncancel(smbus_common_i2c_dev(dev));
}
