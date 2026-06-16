/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/smbus.h>
#include <zephyr/logging/log.h>

#include "smbus_common_operations.h"

LOG_MODULE_REGISTER(emul_smbus, CONFIG_SMBUS_LOG_LEVEL);

struct smbus_emul_config {
	struct smbus_common_i2c_config common;
};

struct smbus_emul_data {
	struct smbus_common_data common; /* must be first - config flags for PEC */
	const struct device *dev;
};

static int smbus_emul_init(const struct device *dev)
{
	const struct smbus_emul_config *config = dev->config;
	struct smbus_emul_data *data = dev->data;

	data->dev = dev;

	if (!device_is_ready(config->common.i2c_dev)) {
		LOG_ERR("%s: I2C device is not ready", dev->name);
		return -ENODEV;
	}

	return 0;
}

static int smbus_emul_configure(const struct device *dev, uint32_t config_value)
{
	struct smbus_emul_data *data = dev->data;

	data->common.config = config_value;
	return 0;
}

static int smbus_emul_get_config(const struct device *dev, uint32_t *config_value)
{
	struct smbus_emul_data *data = dev->data;

	*config_value = data->common.config;
	return 0;
}

static DEVICE_API(smbus, smbus_emul_api) = {
	.configure = smbus_emul_configure,
	.get_config = smbus_emul_get_config,
	.smbus_quick = smbus_common_quick,
	.smbus_byte_write = smbus_common_byte_write,
	.smbus_byte_read = smbus_common_byte_read,
	.smbus_byte_data_write = smbus_common_byte_data_write,
	.smbus_byte_data_read = smbus_common_byte_data_read,
	.smbus_word_data_write = smbus_common_word_data_write,
	.smbus_word_data_read = smbus_common_word_data_read,
	.smbus_pcall = smbus_common_pcall,
	.smbus_block_write = smbus_common_block_write,
	.smbus_block_read = smbus_common_block_read,
	.smbus_cancel = smbus_common_cancel,
	.smbus_uncancel = smbus_common_uncancel,
	.smbus_block_pcall = smbus_common_block_pcall,
	.smbus_smbalert_set_cb = NULL,
	.smbus_smbalert_remove_cb = NULL,
	.smbus_host_notify_set_cb = NULL,
	.smbus_host_notify_remove_cb = NULL,
};

#define DT_DRV_COMPAT zephyr_smbus_emul

#define SMBUS_EMUL_DEVICE_INIT(n)                                                                  \
	static struct smbus_emul_config smbus_emul_config_##n = {                                  \
		.common.i2c_dev = DEVICE_DT_GET(DT_INST_PROP(n, i2c)),                            \
	};                                                                                         \
                                                                                                   \
	static struct smbus_emul_data smbus_emul_data_##n;                                         \
                                                                                                   \
	SMBUS_DEVICE_DT_INST_DEFINE(n, smbus_emul_init, NULL, &smbus_emul_data_##n,               \
				    &smbus_emul_config_##n, POST_KERNEL,                           \
				    CONFIG_SMBUS_INIT_PRIORITY, &smbus_emul_api);

DT_INST_FOREACH_STATUS_OKAY(SMBUS_EMUL_DEVICE_INIT)
