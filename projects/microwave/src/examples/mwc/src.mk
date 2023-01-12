SRCS += $(DRIVERS)/api/no_os_gpio.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_gpio.c \
	$(DRIVERS)/api/no_os_spi.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_spi.c \
	$(DRIVERS)/api/no_os_irq.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_irq.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_rtc.c \
	$(DRIVERS)/api/no_os_uart.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart_stdio.c \
	$(NO-OS)/util/no_os_lf256fifo.c \
	$(NO-OS)/util/no_os_util.c \
	$(NO-OS)/util/no_os_list.c \
	$(NO-OS)/util/no_os_pid.c \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_delay.c

INCS += $(INCLUDE)/no_os_gpio.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_gpio.h \
	$(INCLUDE)/no_os_spi.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_spi.h \
	$(INCLUDE)/no_os_irq.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_irq.h \
	$(INCLUDE)/no_os_rtc.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_rtc.h \
	$(INCLUDE)/no_os_uart.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart_stdio.h \
	$(INCLUDE)/no_os_error.h \
	$(INCLUDE)/no_os_util.h \
	$(INCLUDE)/no_os_units.h \
	$(INCLUDE)/no_os_list.h \
	$(INCLUDE)/no_os_pid.h \
	$(INCLUDE)/no_os_print_log.h \
	$(INCLUDE)/no_os_delay.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_delay.h \
	$(INCLUDE)/no_os_lf256fifo.h

ifneq (y, $(FAKE))
SRCS += $(DRIVERS)/rf-transceiver/hmc630x/hmc630x.c
else
SRCS += $(DRIVERS)/rf-transceiver/hmc630x/hmc630x_fake.c
endif

SRCS +=  $(DRIVERS)/rf-transceiver/hmc630x/iio_hmc630x.c 
INCS += $(DRIVERS)/rf-transceiver/hmc630x/hmc630x.h \
	$(DRIVERS)/rf-transceiver/hmc630x/iio_hmc630x.h


