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
	struct no_os_gpio_desc *areset, *breset;
	uint16_t r, val, phyad;

	ret = no_os_uart_init(&uart_desc, &uart_console_ip);
	if (ret)
		return ret;

	no_os_uart_stdio(uart_desc);

	printf("Hello world\n");

	no_os_gpio_get(&areset, &aphy_reset_gpio_ip);
	no_os_gpio_direction_output(areset, NO_OS_GPIO_LOW);
	no_os_mdelay(10);
	no_os_gpio_direction_output(areset, NO_OS_GPIO_HIGH);
	no_os_mdelay(10);

	no_os_gpio_get(&breset, &bphy_reset_gpio_ip);
	no_os_gpio_direction_output(breset, NO_OS_GPIO_LOW);
	no_os_mdelay(10);
	no_os_gpio_direction_output(breset, NO_OS_GPIO_HIGH);
	no_os_mdelay(10);

	struct mdio_bitbang_init_param mdio_extra_ip = {
		.mdc = mdc_gpio_ip,
		.mdio = mdio_gpio_ip,
	};

	struct no_os_mdio_init_param aphyip = {
		.c45 = true,
		.addr = APHY,
		.ops = &mdio_bitbang_ops,
		.extra = &mdio_extra_ip,
	};
	ret = no_os_mdio_init(&aphy, &aphyip);
	if (ret)
		goto error;
	struct no_os_mdio_init_param bphyip = {
		.c45 = true,
		.addr = BPHY,
		.ops = &mdio_bitbang_ops,
		.extra = &mdio_extra_ip,
	};
	ret = no_os_mdio_init(&bphy, &bphyip);
	if (ret)
		goto error;

	// -------------- Register map ------------
	// for (r = 0; r < 32; r++) {
	// 	ret = no_os_mdio_read(aphy, r, &val);
	// 	printf("R%.2u 0x%.4x\n", r, val);
	// }

	// for (r = 0; r < 32; r++) {
	// 	ret = no_os_mdio_read(bphy, r, &val);
	// 	printf("R%.2u 0x%.4x\n", r, val);
	// }

	// ------------- Set delays on rx to enable back-to-back mode ----------
	// write using emulated c45
	// no_os_mdio_write(aphy, 0x10, 0xff23);
	// no_os_mdio_write(aphy, 0x11, 0xe05);
	// no_os_mdio_read(aphy, 0x11, &val);
	no_os_mdio_write(aphy, NO_OS_MDIO_C45_ADDR(0x1e, 0xff23), 0xe05);
	no_os_mdio_read(aphy, NO_OS_MDIO_C45_ADDR(0x1e, 0xff23), &val);
	printf("A Reg 0xff23: 0x%x\n", val);

	// write using emulated c45
	// no_os_mdio_write(bphy, 0x10, 0xff23);
	// no_os_mdio_write(bphy, 0x11, 0xe05);
	// no_os_mdio_read(bphy, 0x11, &val);
	no_os_mdio_write(bphy, NO_OS_MDIO_C45_ADDR(0x1e, 0xff23), 0xe05);
	no_os_mdio_read(bphy, NO_OS_MDIO_C45_ADDR(0x1e, 0xff23), &val);
	printf("B Reg 0xff23: 0x%x\n", val);

	// ------------- Enable frame generator and checker --------------
	#define ADIN1300_MII_CONTROL		0x0000
	#define ADIN1300_LOOPBACK_MASK		NO_OS_BIT(14)

	#define ADIN1300_PHY_CTRL_1		0x0012
	#define ADIN1300_DIAG_CLK_EN_MASK	NO_OS_BIT(2)

	#define ADIN1300_PHY_CTRL_STATUS_1	0x0013
	#define ADIN1300_LB_ALL_DIG_SEL_MASK	NO_OS_BIT(12)

	#define ADIN1300_RX_ERR_CNT		0x0014

	#define ADIN1300_PHY_STATUS_1		0x001a
	
	#define ADIN1300_PHY_STATUS_2		0x001f

	#define ADIN1300_FC_EN			NO_OS_MDIO_C45_ADDR(0x1e, 0x9403)
	#define ADIN1300_FC_EN_MASK		NO_OS_BIT(0)

	#define ADIN1300_FC_TX_SEL		NO_OS_MDIO_C45_ADDR(0x1e, 0x9407)
	#define ADIN1300_FC_TX_SEL_MASK		NO_OS_BIT(0)

	#define ADIN1300_FC_FRM_CNT_H		NO_OS_MDIO_C45_ADDR(0x1e, 0x940a)
	#define ADIN1300_FC_FRM_CNT_L		NO_OS_MDIO_C45_ADDR(0x1e, 0x940b)
	#define ADIN1300_FC_LEN_ERR_CNT		NO_OS_MDIO_C45_ADDR(0x1e, 0x940c)
	#define ADIN1300_FC_ALGN_ERR_CNT	NO_OS_MDIO_C45_ADDR(0x1e, 0x940d)
	#define ADIN1300_FC_SYMB_ERR_CNT	NO_OS_MDIO_C45_ADDR(0x1e, 0x940e)
	#define ADIN1300_FC_OSZ_ERR_CNT		NO_OS_MDIO_C45_ADDR(0x1e, 0x940f)
	#define ADIN1300_FC_USZ_ERR_CNT		NO_OS_MDIO_C45_ADDR(0x1e, 0x9410)

	#define ADIN1300_FG_EN			NO_OS_MDIO_C45_ADDR(0x1e, 0x9415)
	#define ADIN1300_FG_EN_MASK		NO_OS_BIT(0)

	#define ADIN1300_FG_CNTRL_RSTRT		NO_OS_MDIO_C45_ADDR(0x1e, 0x9416)
	#define ADIN1300_FG_RSTRT_MASK		NO_OS_BIT(3)
	#define ADIN1300_FG_CNTRL_MASK		NO_OS_GENMASK(2, 0)

	#define ADIN1300_FG_FRM_LEN		NO_OS_MDIO_C45_ADDR(0x1e, 0x941a)

	#define ADIN1300_FG_DONE		NO_OS_MDIO_C45_ADDR(0x1e, 0x941e)
	#define ADIN1300_FG_DONE_MASK		NO_OS_BIT(0)

	no_os_mdio_read(aphy, ADIN1300_MII_CONTROL, &val);
	val |= ADIN1300_LOOPBACK_MASK;
	no_os_mdio_write(aphy, ADIN1300_MII_CONTROL, val); // enable loopback mode
		no_os_mdio_read(aphy, ADIN1300_MII_CONTROL, &val);
		printf("ADIN1300_MII_CONTROL: 0x%x\n", val);
	
	no_os_mdio_read(aphy, ADIN1300_PHY_CTRL_STATUS_1, &val);
	val |= ADIN1300_LB_ALL_DIG_SEL_MASK;
	no_os_mdio_write(aphy, ADIN1300_PHY_CTRL_STATUS_1, val); // enable all digital loopback in PHY
		no_os_mdio_read(aphy, ADIN1300_PHY_CTRL_STATUS_1, &val);
		printf("ADIN1300_PHY_CTRL_STATUS_1: 0x%x\n", val);

	no_os_mdio_write(aphy, ADIN1300_PHY_CTRL_1, 0x4); // enable diagnostic clock
		no_os_mdio_read(aphy, ADIN1300_PHY_CTRL_1, &val);
		printf("ADIN1300_PHY_CTRL_1: 0x%x\n", val);
	no_os_mdio_write(aphy, ADIN1300_FG_CNTRL_RSTRT, no_os_field_prep(ADIN1300_FG_RSTRT_MASK | ADIN1300_FG_RSTRT_MASK, 0x9)); // restart frame generator, random number in data field
	no_os_mdio_write(aphy, ADIN1300_FG_EN, no_os_field_prep(ADIN1300_FG_EN_MASK, 1)); // enable frame generator
		no_os_mdio_read(aphy, ADIN1300_FG_EN, &val);
		printf("FG_EN: 0x%x\n", val);

	no_os_mdio_write(bphy, ADIN1300_FC_TX_SEL, no_os_field_prep(ADIN1300_FC_TX_SEL_MASK, 1)); // frame checker checks frames received from MAC
		no_os_mdio_read(bphy, ADIN1300_FC_TX_SEL, &val);
		printf("ADIN1300_FC_TX_SEL: 0x%x\n", val);
	no_os_mdio_write(bphy, ADIN1300_FC_EN, no_os_field_prep(ADIN1300_FC_EN_MASK, 1)); // enable frame checker
		no_os_mdio_read(bphy, ADIN1300_FC_EN, &val);
		printf("ADIN1300_FC_EN: 0x%x\n", val);

		no_os_mdio_read(aphy, ADIN1300_PHY_STATUS_1, &val);
		printf("ADIN1300_PHY_STATUS_1: 0x%x\n", val);

		no_os_mdio_read(aphy, ADIN1300_PHY_STATUS_2, &val);
		printf("ADIN1300_PHY_STATUS_2: 0x%x\n", val);
	while(true) {
		no_os_mdio_read(aphy, ADIN1300_FG_DONE, &val);
		if (val & ADIN1300_FG_DONE_MASK)
			break;
	}

	printf("FG Check done.\n");
	no_os_mdio_read(bphy, ADIN1300_RX_ERR_CNT, &val);
	printf("RX_ERR_CNT: %d\n", val);

	return 0;
error:
	printf("%s returned with %d\n", __func__, ret);
	return ret;
}
