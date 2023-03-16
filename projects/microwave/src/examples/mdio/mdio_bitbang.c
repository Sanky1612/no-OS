#include <errno.h>
#include <stdlib.h>
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "no_os_mdio.h"
#include "mdio_bitbang.h"

#define MDIO_START		1
#define MDIO_START_MASK		NO_OS_GENMASK(31, 30)
#define MDIO_WRITE		1
#define MDIO_READ		2
#define MDIO_READWRITE_MASK	NO_OS_GENMASK(29, 28)
#define MDIO_PHYADDR_MASK	NO_OS_GENMASK(27, 23)
#define MDIO_REGADDR_MASK	NO_OS_GENMASK(22, 18)
#define MDIO_TURNAROUND		2
#define MDIO_TURNAROUND_MASK	NO_OS_GENMASK(17, 16)
#define MDIO_DATA_MASK		NO_OS_GENMASK(15, 0)

struct mdio_bitbang_extra {
        struct no_os_gpio_desc *mdc;
        struct no_os_gpio_desc *mdio;
};

int mdio_bitbang_init(struct no_os_mdio_desc **dev, struct no_os_mdio_init_param *ip)
{
	int ret;
        struct mdio_bitbang_init_param *mbip = ip->extra;

        struct mdio_bitbang_extra *mbe = calloc(1, sizeof(*mbe));
        if (!mbe)
                return -ENOMEM;

	struct no_os_mdio_desc *d = calloc(1, sizeof(*dev));
	if (!d)
		goto error;

	ret = no_os_gpio_get(&mbe->mdc, &mbip->mdc);
	if (ret)
		goto error_0;

	ret = no_os_gpio_get(&mbe->mdio, &mbip->mdio);
	if (ret)
		goto error_1;


	ret = no_os_gpio_direction_output(mbe->mdc, NO_OS_GPIO_LOW);
	if (ret)
		goto error_2;

	ret = no_os_gpio_direction_output(mbe->mdio, NO_OS_GPIO_HIGH);
	if (ret)
		goto error_2;

        d->extra = mbe;
	*dev = d;

	return 0;
error_2:
	no_os_gpio_remove(mbe->mdio);
error_1:
	no_os_gpio_remove(mbe->mdc);
error_0:
	free(d);
error:
        free(mbe);

	return ret;
}

static int mdio_rw22(struct no_os_mdio_desc *dev, uint16_t rw, uint16_t addr)
{
	int i;
	uint32_t frame;
        struct mdio_bitbang_extra *mbe = dev->extra;

	frame = no_os_field_prep(MDIO_START_MASK, MDIO_START) |
		no_os_field_prep(MDIO_READWRITE_MASK, rw) |
		no_os_field_prep(MDIO_PHYADDR_MASK, dev->addr) |
		no_os_field_prep(MDIO_REGADDR_MASK, addr) | 
		no_os_field_prep(MDIO_TURNAROUND_MASK, MDIO_TURNAROUND);

	// preamble
	for (i = 31; i >= 0; i--) {
		no_os_gpio_direction_output(mbe->mdio, NO_OS_GPIO_HIGH);
		no_os_gpio_set_value(mbe->mdc, NO_OS_GPIO_HIGH);
		no_os_gpio_set_value(mbe->mdc, NO_OS_GPIO_LOW);
	}

	// start, read, phyaddr, regaddr
	for (i = 31; i >= 16; i--) {
		no_os_gpio_set_value(mbe->mdio, (bool)(frame & (1 << i)));
		no_os_gpio_set_value(mbe->mdc, NO_OS_GPIO_HIGH);
		no_os_gpio_set_value(mbe->mdc, NO_OS_GPIO_LOW);
	}

	return 0;
}

int mdio_bitbang_write(struct no_os_mdio_desc *dev, uint16_t addr, uint16_t in)
{
	int i;
        struct mdio_bitbang_extra *extra = dev->extra;

	mdio_rw22(dev, MDIO_WRITE, addr);

	// register data
	for (i = 15; i >= 0; i--) {
		no_os_gpio_set_value(extra->mdio, (in >> i) & 0x1);
		no_os_gpio_set_value(extra->mdc, NO_OS_GPIO_HIGH);
		no_os_gpio_set_value(extra->mdc, NO_OS_GPIO_LOW);
	}

	return 0;
}

int mdio_bitbang_read(struct no_os_mdio_desc *dev, uint16_t addr, uint16_t *out)
{
	int i;
	uint8_t state;
        struct mdio_bitbang_extra *extra = dev->extra;
	*out = 0;

	mdio_rw22(dev, MDIO_READ, addr);

	no_os_gpio_direction_input(extra->mdio);

	// register data
	for (i = 15; i >= 0; i--) {
		no_os_gpio_get_value(extra->mdio, &state);
		if (state)
			*out |= 1;
		if (i)
			*out <<= 1;
		no_os_gpio_set_value(extra->mdc, NO_OS_GPIO_HIGH);
		no_os_gpio_set_value(extra->mdc, NO_OS_GPIO_LOW);
	}

	return 0;
}

int mdio_bitbang_remove(struct no_os_mdio_desc *dev)
{
        return 0;
}

struct no_os_mdio_ops mdio_bitbang_ops = {
	.init = mdio_bitbang_init,
        .write = mdio_bitbang_write,
	.read = mdio_bitbang_read,
	.remove = mdio_bitbang_remove,
};