#ifndef _MDIO_BITBANG_H
#define _MDIO_BITBANG_H

#include "no_os_gpio.h"

struct mdio_bitbang_init_param {
        struct no_os_gpio_init_param mdc;
        struct no_os_gpio_init_param mdio;
};

extern struct no_os_mdio_ops mdio_bitbang_ops;

#endif
