#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "no_os_uart.h"
#include "no_os_gpio.h"
#include "maxim_irq.h"
#include "maxim_gpio.h"
#include "maxim_uart.h"
#include "maxim_uart_stdio.h"

#define UART_DEVICE_ID  0
#define UART_BAUDRATE   115200
#define UART_STOPBITS   NO_OS_UART_STOP_2_BIT
#define UART_IRQ_ID     UART0_IRQn
#define INTC_DEVICE_ID  0

#if (TARGET_NUM==32650)
#define RESET_PORT      1
#define RESET_PIN       27

#define EN_PORT         1
#define EN_PIN          23

#define CLK_PORT        1
#define CLK_PIN         26

#define DATA_PORT       1
#define DATA_PIN        29

#define SCANOUT_TX_PORT 1
#define SCANOUT_TX_PIN  28

#define SCANOUT_RX_PORT 1
#define SCANOUT_RX_PIN  24

#define MDC_PORT        2
#define MDC_PIN         17

#define MDIO_PORT       2
#define MDIO_PIN        18

#define PHY_RESET_PORT  2
#define PHY_RESET_PIN   13

#define BPHY_RESET_PORT  2
#define BPHY_RESET_PIN   15

// RevA
#define RESET_ADIN1300_PORT	1
#define RESET_ADIN1300_PIN	23

#define RESET_MAX24287_PORT	1
#define RESET_MAX24287_PIN	24

#define MDIO_ADIN1300_PORT	1
#define MDIO_ADIN1300_PIN	4

#define MDC_ADIN1300_PORT	1
#define MDC_ADIN1300_PIN	5

#define MDIO_MAX24287_PORT	1
#define MDIO_MAX24287_PIN	6

#define MDC_MAX24287_PORT	1
#define MDC_MAX24287_PIN	7

#elif (TARGET_NUM==78000)
#define RESET_PORT      0
#define RESET_PIN       9

#define EN_PORT         0
#define EN_PIN          11

#define CLK_PORT        0
#define CLK_PIN         7

#define DATA_PORT       0
#define DATA_PIN        5

#define SCANOUT_TX_PORT 0
#define SCANOUT_TX_PIN  6

#define SCANOUT_RX_PORT 0
#define SCANOUT_RX_PIN  8

#define MDC_PORT        0
#define MDC_PIN         17

#define MDIO_PORT       0
#define MDIO_PIN        16

#define PHY_RESET_PORT  3
#define PHY_RESET_PIN   1

#define BPHY_RESET_PORT  0
#define BPHY_RESET_PIN   19
#endif  

extern struct no_os_uart_init_param uart_console_ip;
extern struct no_os_gpio_init_param reset_gpio_ip;
extern struct no_os_gpio_init_param en_gpio_ip;
extern struct no_os_gpio_init_param clk_gpio_ip;
extern struct no_os_gpio_init_param data_gpio_ip;
extern struct no_os_gpio_init_param scanout_tx_gpio_ip;
extern struct no_os_gpio_init_param scanout_rx_gpio_ip;
extern struct no_os_gpio_init_param mdc_gpio_ip;
extern struct no_os_gpio_init_param mdio_gpio_ip;
extern struct no_os_gpio_init_param phy_reset_gpio_ip;
extern struct no_os_gpio_init_param bphy_reset_gpio_ip;

// RevA
extern struct no_os_gpio_init_param reset_adin1300_gpio_ip;
extern struct no_os_gpio_init_param reset_max24287_gpio_ip;
extern struct no_os_gpio_init_param mdc_adin1300_gpio_ip;
extern struct no_os_gpio_init_param mdio_adin1300_gpio_ip;
extern struct no_os_gpio_init_param mdc_max24287_gpio_ip;
extern struct no_os_gpio_init_param mdio_max24287_gpio_ip;

#endif /* __PARAMETERS_H__ */
