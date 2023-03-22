#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_print_log.h"
#include "no_os_util.h"
#include "parameters.h"
#include "no_os_mdio.h"
#include "mdio_bitbang.h"

#define APHY	0x1
#define BPHY	0x2
int regtest(struct no_os_mdio_desc *mdio, uint16_t r, uint16_t pattern)
{
	uint16_t val;
	no_os_mdio_write(mdio, r, pattern);
	no_os_mdio_read(mdio, r, &val);
	if (val != pattern) {
		printf("Mismatch! Got 0x%x expected 0x%x\n", val, pattern);
		return -EFAULT;
	}

	return 0;
}

int main()
{
	int ret;
	struct no_os_uart_desc *uart_desc;
	struct no_os_mdio_desc *aphy, *bphy;
	uint16_t r, val, phyad;

	ret = no_os_uart_init(&uart_desc, &uart_console_ip);
	if (ret)
		return ret;

	no_os_uart_stdio(uart_desc);

	printf("Hello world\n");

	struct mdio_bitbang_init_param mdio_extra_ip = {
		.mdc = mdc_gpio_ip,
		.mdio = mdio_gpio_ip,
	};

	struct no_os_mdio_init_param aphyip = {
		.addr = APHY,
		.ops = &mdio_bitbang_ops,
		.extra = &mdio_extra_ip,
	};
	ret = no_os_mdio_init(&aphy, &aphyip);
	if (ret)
		goto error;
	struct no_os_mdio_init_param bphyip = {
		.addr = BPHY,
		.ops = &mdio_bitbang_ops,
		.extra = &mdio_extra_ip,
	};
	ret = no_os_mdio_init(&bphy, &bphyip);
	if (ret)
		goto error;

	for (r = 0; r < 32; r++) {
		ret = no_os_mdio_read(aphy, r, &val);
		printf("R%.2u 0x%.4x\n", r, val);
	}

	for (r = 0; r < 32; r++) {
		ret = no_os_mdio_read(bphy, r, &val);
		printf("R%.2u 0x%.4x\n", r, val);
	}

	aphy->c45 = true;
	// no_os_mdio_write(aphy, 0x10, 0xff23);
	// no_os_mdio_write(aphy, 0x11, 0xe05);
	// no_os_mdio_read(aphy, 0x11, &val);
	no_os_mdio_write(aphy, NO_OS_MDIO_C45_ADDR(0x1e, 0xff23), 0xe05);
	no_os_mdio_read(aphy, NO_OS_MDIO_C45_ADDR(0x1e, 0xff23), &val);
	printf("A Reg 0xff23: 0x%x\n", val);

	bphy->c45 = true;
	// no_os_mdio_write(bphy, 0x10, 0xff23);
	// no_os_mdio_write(bphy, 0x11, 0xe05);
	// no_os_mdio_read(bphy, 0x11, &val);
	no_os_mdio_write(bphy, NO_OS_MDIO_C45_ADDR(0x1e, 0xff23), 0xe05);
	no_os_mdio_read(bphy, NO_OS_MDIO_C45_ADDR(0x1e, 0xff23), &val);
	printf("B Reg 0xff23: 0x%x\n", val);

	return 0;
error:
	printf("%s returned with %d\n", __func__, ret);
	return ret;
}
