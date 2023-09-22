/***************************************************************************//**
 *   @file   max22190.h
 *   @brief  Header file of MAX22190 Driver.
 *   @author Radu Sabau (radu.sabau@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef _MAX22190_H
#define _MAX22190_H

#include <stdint.h>
#include <stdbool.h>
#include "no_os_gpio.h"
#include "no_os_spi.h"
#include "no_os_util.h"

#define MAX22190_FRAME_SIZE		2
#define MAX22190_CHANNELS		8

#define MAX22190_WIRE_BREAK_REG		0x0
#define MAX22190_DIGITAL_INPUT_REG	0x2
#define MAX22190_FAULT1_REG		0x4
#define MAX22190_FILTER_IN1_REG		0x6
#define MAX22190_FILTER_IN2_REG		0x8
#define MAX22190_FILTER_IN3_REG		0xA
#define MAX22190_FILTER_IN4_REG		0xC
#define MAX22190_FILTER_IN5_REG		0xE
#define MAX22190_FILTER_IN6_REG		0x10
#define MAX22190_FILTER_IN7_REG		0x12
#define MAX22190_FILTER_IN8_REG		0x14
#define MAX22190_CONFIG_REG		0x18
#define MAX22190_INPUT_EN_REG		0x1A
#define MAX22190_FAULT2_REG		0x1C
#define MAX22190_FAULT2_EN_REG		0x1E
#define MAX22190_GPO_REG		0x22
#define MAX22190_FAULT1_EN_REG		0x24

#define MAX22190_NO_OP_REG		0x26

// for filter set/get
#define MAX22190_DELAY_MASK		NO_OS_GENMASK(2, 0)
#define MAX22190_FBP_MASK		NO_OS_BIT(3)
#define MAX22190_WBE_MASK		NO_OS_BIT(4)
#define MAX22190_RW_MASK		NO_OS_BIT(7)
#define MAX22190_ADDR_MASK		NO_OS_GENMASK(6, 0)

struct max22190_init_param {
	struct no_os_spi_init_param *comm_param;
	struct no_os_gpio_init_param *en_gpio_param;
	bool crc_en;
};

struct max22190_desc {
	struct no_os_spi_init_param *comm_desc;
	struct no_os_gpio_desc *en_gpio;
	uint8_t buff[MAX22190_FRAME_SIZE + 1];
	bool crc_en;
};

enum max22190_delay {
	MAX22190_DELAY_50US,s
	MAX22190_DELAY_100US,
	MAX22190_DELAY_400US,
	MAX22190_DELAY_800US,
	MAX22190_DELAY_1800US,
	MAX22190_DELAY_3200US,
	MAX22190_DELAY_12800US,
	MAX22190_DELAY_20MS
};

enum max22190_ch_state {
	MAX22190_CH_OFF,
	MAX22190_CH_ON
};

/** Implement fault functions */
static uint8_t max22190_crc(uint8_t, uint8_t, uint8_t);

/** Set filter values for specific channel. */
int max22190_filter_set(struct max22190_desc *, uint32_t, uint32_t,
			uint32_t, enum max22190_delay);

/** Get filter values for specific channel. */
int max22190_filter_get(struct max22190_desc *, uint32_t, uint32_t *,
			uint32_t *, enum max22190_delay);

/** Switch the state of a specific channel to disabled/enabled. */
int max22190_chan_state(struct max22190_desc *, uint32_t, uint32_t);

/** Read the register of the MAX22190 device. */
int max22190_reg_read(struct max22190_desc *, uint32_t, uint32_t *);

/** Write the register of the MAX22190 device. */
int max22190_reg_write(struct max22190_desc *, uint32_t, uint32_t);

/** Update the register of the MAX22190 device. */
int max22190_reg_update(struct max22190_desc *, uint32_t, uint32_t);

/** Initialize and configure the MAX14916 device. */
int max22190_init(struct max22190_desc **, struct max22190_init_param *);

/** Free the resources allocated by the initalization function. */
int max22190_remove(struct max22190_desc *);

#endif
