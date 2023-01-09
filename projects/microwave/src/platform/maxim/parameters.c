#include "parameters.h"

struct max_uart_init_param uart_extra_ip = {
	.flow = UART_FLOW_DIS
};

struct no_os_uart_init_param uart_console_ip = {
	.irq_id = UART_IRQ_ID,
	.asynchronous_rx = true,
	.device_id = UART_DEVICE_ID,
	.baud_rate = UART_BAUDRATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = UART_STOPBITS,
	.platform_ops = &max_uart_ops,
	.extra = &uart_extra_ip,
};

struct max_gpio_init_param xgpio = {
	.vssel = MXC_GPIO_VSSEL_VDDIOH,
};

struct no_os_gpio_init_param reset_gpio_ip = {
	.port = RESET_PORT,
	.number = RESET_PIN,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};

struct no_os_gpio_init_param en_gpio_ip = {
	.port = EN_PORT,
	.number = EN_PIN,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};

struct no_os_gpio_init_param clk_gpio_ip = {
	.port = CLK_PORT,
	.number = CLK_PIN,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};

struct no_os_gpio_init_param data_gpio_ip = {
	.port = DATA_PORT,
	.number = DATA_PIN,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};

struct no_os_gpio_init_param scanout_tx_gpio_ip = {
	.port = SCANOUT_TX_PORT,
	.number = SCANOUT_TX_PIN,
	.pull = NO_OS_PULL_UP,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};

struct no_os_gpio_init_param scanout_rx_gpio_ip = {
	.port = SCANOUT_RX_PORT,
	.number = SCANOUT_RX_PIN,
	.pull = NO_OS_PULL_UP,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};

struct no_os_gpio_init_param mdc_gpio_ip = {
	.port = MDC_PORT,
	.number = MDC_PIN,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};

struct no_os_gpio_init_param mdio_gpio_ip = {
	.port = MDIO_PORT,
	.number = MDIO_PIN,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};

struct no_os_gpio_init_param phy_reset_gpio_ip = {
	.port = PHY_RESET_PORT,
	.number = PHY_RESET_PIN,
	.platform_ops = &max_gpio_ops,
	.extra = &xgpio,
};