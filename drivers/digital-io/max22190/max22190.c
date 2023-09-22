#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "max22190.h"
#include "no_os_util.h"
#include "no_os_alloc.h"

/**
 * @brief Compute the CRC5 value for an array of bytes when writing to MAX14906
 * @param data - array of data to encode
 * @return the resulted CRC5
 */
static uint8_t max22190_crc(uint8_t data2, uint8_t data1, uint8_t data0)
{
	int length = 19;
	uint8_t crc_init = 0x7;
	uint8_t crc_poly = 0x35;
	uint8_t crc_step, tmp;
	int i;

	/**
	 * This is the C custom implementation of CRC function for MAX22190, and
	 * can be found here:
	 * https://www.analog.com/en/design-notes/guidelines-to-implement-crc-algorithm.html
	*/
	uint32_t datainput = (uint32_t)((data2 << 16) + (data1 << 8) + data0);

	datainput = (datainput & 0xFFFFE0) + crc_init;

	tmp = (uint8_t)((datainput & 0xFC0000) >> 18);

	if ((tmp & 0x20) == 0x20)
		crc_step = (uint8_t)(tmp ^ crc_poly);
	else
		crc_step = tmp;

	for (i = 0; i < length - 1; i++) {
		tmp = (uint8_t)(((crc_step & 0x1F) << 1) +
				((datainput >> (length -2 - i)) & 0x01));

		if ((tmp & 0x20) == 0x20)
			crc_step = (uint8_t)(tmp ^ crc_poly);
		else
			crc_step = tmp;
	}

	return (uint8_t)(crc_step & 0x1F);
}

int max22190_reg_read(struct max22190_desc *desc, uint32_t addr, uint32_t *val)
{
	struct no_os_spi_msg xfer = {
		.tx_buff = desc->buff,
		.rx_buff = desc->buff,
		.bytes_number = MAX22190_FRAME_SIZE,
		.cs_change = 1,
	};
	uint8_t crc;
	int ret;

	if (desc->crc_en)
		xfer.bytes_number++;

	memset(desc->buff, 0, xfer.bytes_number);
	desc->buff[0] = no_os_field_prep(MAX22190_ADDR_MASK, addr) |
			no_os_field_prep(MAX22190_RW_MASK, 0);

	ret = no_os_spi_transfer(desc->comm_desc, &xfer, 1);
	if (ret)
		return ret;

	if (desc->crc_en) {
		if (max22190_crc(desc->buff[2], 0, desc->buff[0]))
			return -EINVAL;
	}

	*val = desc->buff[1];

	return 0;
}

int max22190_reg_write(struct max22190_desc *desc, uint32_t addr, uint32_t val)
{
	struct no_os_spi_msg xfer = {
		.tx_buff = desc->buff,
		.bytes_number = MAX22190_FRAME_SIZE,
		.cs_change = 1,
	};

	desc->buff[0] = no_os_field_prep(MAX22190_ADDR_MASK, addr);
	no_os_field_prep(MAX22190_RW_MASK, 1);
	desc->buff[1] = val;

	if (desc->crc_en) {
		xfer.bytes_number++;
		desc->buff[2] = max22190_crc(0, desc->buff[1], desc->buff[0]);
	}

	return no_os_spi_transfer(desc->comm_desc, &xfer, 1);
}

int max22190_reg_update(struct max22190_desc *desc, uint32_t addr,
			uint32_t mask, uint32_t val)
{
	int ret;
	uint32_t reg_val = 0;

	ret = max22190_reg_read(desc, addr, &reg_val);
	if (ret)
		return ret;

	reg_val &= ~mask;
	reg_val |= mask & val;

	return max22190_reg_write(desc, addr, reg_val);
}

int max22190_init(struct max22190_desc **desc,
		  struct max22190_init_param *param)
{
	struct max22190_desc *descriptor;
	uint32_t reg_val;
	int ret;
	int i;

	descriptor = no_os_calloc(1, sizeof(*descriptor));
	if (!descriptor)
		return -ENOMEM;

	ret = no_os_spi_init(&descriptor->comm_desc, param->comm_param);
	if (ret)
		goto err;

	descriptor->crc_en = param->crc_en;

	ret = no_os_gpio_get_optional(&descriptor->en_gpio,
				      param->en_gpio_param);
	if (ret)
		goto spi_err;

	if (descriptor->en_gpio) {
		ret = no_os_gpio_set_value(descriptor->en_gpio,
					   NO_OS_GPIO_HIGH);
		if (ret)
			goto gpio_err;
	}

	ret = max22190_reg_read(descriptor, MAX22190_WIRE_BREAK_REG, &reg_val);
	if (ret)
		goto gpio_err;

	ret = max22190_reg_read(descriptor, MAX22190_FAULT2_REG, &reg_val);
	if (ret)
		goto gpio_err;

	for (i = 0; i < MAX22190_CHANNELS; i++) {
		ret = max22190_chan_state(descriptor, i, MAX22190_CH_ON);
		if (ret)
			goto gpio_err;
	}

	*desc = descriptor;

	return 0;

gpio_err:
	no_os_gpio_remove(descriptor->en_gpio);
spi_err:
	no_os_spi_remove(descriptor->comm_desc);
err:
	no_os_free(descriptor);

	return ret;
}

int max22190_remove(struct max22190_desc *desc)
{
	int ret;
	int i;

	if (!desc)
		return -ENODEV;

	for(i = 0; i < MAX22190_CHANNELS; i++) {
		ret = max22190_ch_state(desc, i, MAX22190_CH_OFF);
		if (ret)
			return ret;
	}

	no_os_spi_remove(desc->comm_desc);

	if (desc->en_gpio)
		no_os_gpio_remove(desc->en_gpio);

	no_os_free(desc);

	return 0;
}
