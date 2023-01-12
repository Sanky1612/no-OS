/***************************************************************************//**
 *   @file   hmc630x_fake.c
 *   @brief  hmc6300 and hmc6301 device fake driver implementation.
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2022(c) Analog Devices, Inc.
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "hmc630x.h"
#include "no_os_delay.h"

#define HMC630X_ARRAY_ADDRESS_MASK NO_OS_GENMASK(13, 8)
#define HMC630X_RW_MASK NO_OS_BIT(14)
#define HMC630X_ADDRESS_MASK NO_OS_GENMASK(17, 15)
#define HMC6300_ADDRESS 0x6
#define HMC6301_ADDRESS 0x7
#define HMC630X_FRAME_SIZE 18

#define HMC6300_BITBANG_DELAY_US 1
#define HMC6300_SETTLING_DELAY_MS 1

/* VCO frequencies computed by hmc6300_init() based on reference clock. */
struct hmc630x_vco {
	uint64_t *freqs;
	uint8_t *fbdiv;
	uint8_t entries;
};

/* Device descriptor created by hmc6300_init() and used by the rest of the driver API. */
struct hmc630x_dev {
	enum hmc630x_type type;
	struct hmc630x_vco vco;
	uint64_t fake_freq;
	uint8_t fake_regmap[32];
};

/* Default values for registers as listed in datasheet, written to device at startup. */
static const uint8_t hmc6300_default_regmap[] = {
	0x00, 0x4a, 0xf6, 0xf6, 0x00, 0xbf, 0x6c, 0x0f,
	0x8f, 0x00, 0x53, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x36, 0xbb, 0x46, 0x02, 0x22, 0x12, 0x00, 0x62
};
static const uint8_t hmc6301_default_regmap[] = {
	0x00, 0x10, 0x00, 0x03, 0x9f, 0x0f, 0xbf, 0x6d,
	0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x36, 0xbb, 0x46, 0x02, 0x22, 0x12, 0x00, 0x62
};

/* Generate pairs of valid VCO frequencies vs their corresponding FBDIV values. */
static int _hmc630x_generate_vco(struct hmc630x_dev *d, uint64_t f_start,
				 uint64_t f_step, uint8_t fbdiv_start, uint8_t entries)
{
	int ret, e;

	d->vco.freqs = (uint64_t *)calloc(entries, sizeof(*d->vco.freqs));
	if (!d->vco.freqs)
		return -ENOMEM;

	d->vco.fbdiv = (uint8_t *)calloc(entries, sizeof(*d->vco.fbdiv));
	if (!d->vco.fbdiv) {
		ret = -ENOMEM;
		goto error;
	}

	for (e = 0; e < entries; e++) {
		d->vco.freqs[e] = f_start + e * f_step;
		d->vco.fbdiv[e] = fbdiv_start + e;
	}

	d->vco.entries = entries;

	return 0;
error:
	free(d->vco.freqs);
	return ret;
}

/* Create hmc6300 device and initialize it from init params. */
int hmc630x_init(struct hmc630x_dev **dev, struct hmc630x_init_param *init)
{
	int ret;
	struct hmc630x_dev *d;
	const uint8_t *regmap;

	if (!dev || !init)
		return -EINVAL;

	d = (struct hmc630x_dev *) calloc(1, sizeof(*d));
	if (!d)
		return -ENOMEM;

	switch (init->type) {
	case HMC6300:
		regmap = hmc6300_default_regmap;
		break;
	case HMC6301:
		regmap = hmc6301_default_regmap;
		break;
	default:
		ret = -EINVAL;
		goto error_0;
		break;
	}
	d->type = init->type;
	memcpy(d->fake_regmap, regmap, 24);

	switch (init->ref_clk) {
	case HMC6300_REF_CLK_71p42857MHz:
		ret = _hmc630x_generate_vco(d, 56500000000, 250000000, 0x22, 31);
		break;
	case HMC6300_REF_CLK_75MHz:
		ret = _hmc630x_generate_vco(d, 55125000000, 262500000, 0x12, 43);
		break;
	case HMC6300_REF_CLK_142p8571MHz:
		ret = _hmc630x_generate_vco(d, 56500000000, 500000000, 0x11, 16);
		break;
	case HMC6300_REF_CLK_154p2857MHz:
		ret = _hmc630x_generate_vco(d, 57240000000, 540000000, 0x0a, 17);
		break;
	default:
		ret = -EINVAL;
	}

	if (ret)
		goto error_0;

	*dev = d;
	return 0;

error_0:
	free(d);
	return ret;
}

/* Remove the hmc6300 device by deallocating resources. */
int hmc630x_remove(struct hmc630x_dev *dev)
{
	int ret;

	free(dev->vco.freqs);
	dev->vco.freqs = NULL;
	free(dev->vco.fbdiv);
	dev->vco.fbdiv = NULL;

	free(dev);
	dev = NULL;

	return 0;
}

/* Write a device row using GPIO bit-banging. */
int hmc630x_write_row(struct hmc630x_dev *dev, uint8_t row, uint8_t val)
{
	dev->fake_regmap[row] = val;
	return 0;
}

/* Read a device row using GPIO bit-banging. */
int hmc630x_read_row(struct hmc630x_dev *dev, uint8_t row, uint8_t *val)
{
	*val = dev->fake_regmap[row];
	return 0;
}

/* Write the write-accessible registers of the register map. */
int hmc630x_write_regmap(struct hmc630x_dev *dev, const uint8_t *regmap)
{
	int ret, r, skip1, skip2;
	uint8_t reg;

	switch(dev->type) {
	case HMC6301:
		r = 0;
		skip1 = 10;
		skip2 = 15;
		break;
	default:
	case HMC6300:
		r = 1;
		skip1 = 13;
		skip2 = 15;
		break;
	};

	for (; r < 24; r++) {
		if (r >= skip1 && r <= skip2)
			continue;
		reg = regmap[r];
		ret = hmc630x_write_row(dev, r, reg);
		if (ret < 0)
			return ret;
		ret = hmc630x_read_row(dev, r, &reg);
		if (ret < 0)
			return ret;
		if (reg != regmap[r])
			return -EFAULT;
	}

	return 0;
}

/* Write a device parameter (bit or bit-field of a certain row). */
int hmc630x_write(struct hmc630x_dev *dev, uint16_t param, uint8_t value)
{
	int ret;
	uint8_t reg;
	uint8_t row = HMC630X_ROW(param);
	uint8_t mask = HMC630X_MASK(param);

	ret = hmc630x_read_row(dev, row, &reg);
	if (ret)
		return ret;

	reg &= ~mask;
	reg |= no_os_field_prep(mask, value);
	return hmc630x_write_row(dev, row, reg);
}

/* Read a device parameter (bit or bit-field of a certain row). */
int hmc630x_read(struct hmc630x_dev *dev, uint16_t param, uint8_t *value)
{
	int ret;
	uint8_t reg;

	if (!value)
		return -EINVAL;

	ret = hmc630x_read_row(dev, HMC630X_ROW(param), &reg);
	if (ret)
		return ret;

	*value = no_os_field_get(HMC630X_MASK(param), reg);
	return 0;
}

/* Read the whole register map and store it into a 32-byte output buffer. */
int hmc630x_read_regmap(struct hmc630x_dev *dev, uint8_t *regmap)
{
	int ret, skip1, skip2;
	uint8_t r;

	switch(dev->type) {
	case HMC6301:
		r = 0;
		skip1 = 10;
		skip2 = 15;
		break;
	default:
	case HMC6300:
		r = 1;
		skip1 = 13;
		skip2 = 15;
		break;
	};

	for (; r < 32; r++) {
		regmap[r] = 0;

		if (r >= skip1 && r <= skip2)
			continue;

		ret = hmc630x_read_row(dev, r, &regmap[r]);
		if (ret)
			break;
	}

	return ret;
}

/* Enable temperature sensor. */
int hmc630x_enable_temp(struct hmc630x_dev *dev, bool enable)
{
	return 0;
}

/* Get a temperature reading. */
int hmc630x_get_temp(struct hmc630x_dev *dev, uint8_t *temp)
{
	return 7;
}

/* Enable FSK/MSK modulation inputs. */
int hmc6300_enable_fm(struct hmc630x_dev *dev, bool enable)
{
	return 0;
}

/* Power On/Off the chip. */
int hmc630x_enable(struct hmc630x_dev *dev, bool enable)
{
	return 0;
}

/* Set the IF attenuation in steps from 0 (highest gain) to 15.
 * For digital setting to dB correlation, see the datasheet.
 */
int hmc630x_set_if_attn(struct hmc630x_dev *dev, uint8_t attn)
{
	return 0;
}

/* Set the RF attenuation in steps from 0 (highest gain) to 15.
 * For digital setting to dB correlation, see the datasheet.
 */
int hmc6300_set_rf_attn(struct hmc630x_dev *dev, uint8_t attn)
{
	return 0;
}

/* Set the VCO frequency (Hz). */
int hmc630x_set_vco(struct hmc630x_dev *dev, uint64_t frequency)
{
	dev->fake_freq = frequency;
	return 0;
}

/* Get the available VCO frequencies in the avail array with avail_num entries. */
int hmc630x_get_avail_vco(struct hmc630x_dev *dev, const uint64_t **avail,
			  uint8_t *avail_num)
{
	if (!dev || !avail || !avail_num)
		return -EINVAL;

	*avail = dev->vco.freqs;
	*avail_num = dev->vco.entries;

	return 0;
}

/* Set the receiver LNA gain. */
int hmc6301_set_lna_gain(struct hmc630x_dev *dev, enum hmc6301_lna_gain gain)
{
	return 0;
}

/* Set the receiver baseband attenuation. */
int hmc6301_set_bb_attn(struct hmc630x_dev *dev, enum hmc6301_bb_attn attn1,
			enum hmc6301_bb_attn attn2)
{
	return 0;
}

/* Set the receiver fine baseband attenuation (0-5 dB). */
int hmc6301_set_bb_attn_fine(struct hmc630x_dev *dev, enum hmc6301_bb_attn_fine attn_i,
			     enum hmc6301_bb_attn_fine attn_q)
{
	return 0;
}

/* Set the low-pass corner and high-pass corner of the baseband amplifiers. */
int hmc6301_set_bb_lpc_hpc(struct hmc630x_dev *dev, enum hmc6301_bb_lpc lpc,
			   enum hmc6301_bb_hpc hpc)
{
	return 0;
}
