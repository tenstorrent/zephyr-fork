/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Extended operations of the generic MSPI NOR flash driver.
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_FLASH_MSPI_NOR_H_
#define ZEPHYR_INCLUDE_DRIVERS_FLASH_MSPI_NOR_H_

#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/mspi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extended operations of the jedec,mspi-nor flash driver.
 */
enum flash_mspi_nor_ex_ops {
	/**
	 * Set the Page Program command and IO mode at runtime.
	 *
	 * Overrides the write-command and write-io-mode devicetree
	 * properties, e.g. after identifying at runtime which of several
	 * second-source flash chips is populated. Must be called after
	 * the driver has initialized and before the first write.
	 *
	 * As input it takes a pointer to a @ref flash_mspi_nor_pp
	 * structure. No output.
	 */
	FLASH_MSPI_NOR_EX_OP_SET_PP = FLASH_EX_OP_VENDOR_BASE,

	/**
	 * Set the number of RX dummy cycles for read commands at runtime.
	 *
	 * Overrides the rx-dummy devicetree property, e.g. to match the
	 * dummy-cycle configuration of the flash chip identified at
	 * runtime. Must be called after the driver has initialized and
	 * before the first read.
	 *
	 * The cycle count is passed directly in the input argument, not
	 * through a pointer. No output.
	 */
	FLASH_MSPI_NOR_EX_OP_SET_RX_DUMMY,
};

/** @brief Input for @ref FLASH_MSPI_NOR_EX_OP_SET_PP. */
struct flash_mspi_nor_pp {
	/** Page Program command opcode. */
	uint8_t cmd;
	/** IO mode for the Page Program command. */
	enum mspi_io_mode io_mode;
};

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_FLASH_MSPI_NOR_H_ */
