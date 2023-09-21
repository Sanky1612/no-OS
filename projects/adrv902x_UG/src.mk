################################################################################
#									       #
#     Shared variables:							       #
#	- PROJECT							       #
#	- DRIVERS							       #
#	- INCLUDE							       #
#	- PLATFORM_DRIVERS						       #
#	- NO-OS								       #
#									       #
################################################################################

# Uncomment to select the profile

#PROFILE =  tx_bw100_orx_bw100_rx_bw100
#PROFILE = tx_bw100_orx_bw100_rx_bw40
PROFILE = tx_bw200_orx_bw200_rx_bw100
SRCS +=	$(PROJECT)/src/app/headless.c \
	$(PROJECT)/src/hal/no_os_platform.c
SRCS += $(DRIVERS)/axi_core/axi_adc_core/axi_adc_core.c \
	$(DRIVERS)/axi_core/axi_dac_core/axi_dac_core.c \
	$(DRIVERS)/axi_core/axi_dmac/axi_dmac.c \
	$(DRIVERS)/axi_core/jesd204/axi_jesd204_rx.c \
	$(DRIVERS)/axi_core/jesd204/axi_jesd204_tx.c \
	$(DRIVERS)/axi_core/jesd204/jesd204_clk.c \
	$(DRIVERS)/frequency/ad9528/ad9528.c \
	$(NO-OS)/util/no_os_util.c \
	$(NO-OS)/util/no_os_clk.c \
	$(NO-OS)/util/no_os_alloc.c \
	$(NO-OS)/util/no_os_mutex.c\
	$(DRIVERS)/api/no_os_spi.c \
	$(DRIVERS)/api/no_os_gpio.c \
	$(NO-OS)/jesd204/jesd204-core.c \
	$(NO-OS)/jesd204/jesd204-fsm.c
SRCS += $(DRIVERS)/axi_core/jesd204/xilinx_transceiver.c \
	$(DRIVERS)/axi_core/jesd204/axi_adxcvr.c \
	$(DRIVERS)/axi_core/clk_axi_clkgen/clk_axi_clkgen.c \
	$(PLATFORM_DRIVERS)/xilinx_axi_io.c \
	$(PLATFORM_DRIVERS)/xilinx_delay.c \
	$(PLATFORM_DRIVERS)/xilinx_spi.c \
	$(PLATFORM_DRIVERS)/xilinx_gpio.c

# Madura API sources
SRC_DIRS += $(DRIVERS)/rf-transceiver/madura

INCS +=	$(PROJECT)/src/app/app_config.h \
	$(PROJECT)/src/app/parameters.h \
	$(PROJECT)/src/app/ADRV9025_RxGainTable.h \
	$(PROJECT)/src/app/ADRV9025_TxAttenTable.h \
	$(PROJECT)/src/hal/no_os_platform.h \
	$(PROJECT)/src/firmware/ADRV9025_DPDCORE_FW.h \
	$(PROJECT)/src/firmware/ADRV9025_FW.h \
	$(PROJECT)/src/firmware/stream_image_6E3E00EFB74FE7D465FA88A171B81B8F.h \
	$(PROJECT)/src/firmware/ActiveUseCase_profile.h \
	$(PROJECT)/src/firmware/ActiveUtilInit_profile.h
INCS += $(DRIVERS)/axi_core/axi_adc_core/axi_adc_core.h \
	$(DRIVERS)/axi_core/axi_dac_core/axi_dac_core.h \
	$(DRIVERS)/axi_core/axi_dmac/axi_dmac.h \
	$(DRIVERS)/axi_core/jesd204/axi_jesd204_rx.h \
	$(DRIVERS)/axi_core/jesd204/axi_jesd204_tx.h \
	$(DRIVERS)/axi_core/jesd204/jesd204_clk.h \
	$(DRIVERS)/frequency/ad9528/ad9528.h
INCS += $(DRIVERS)/axi_core/jesd204/xilinx_transceiver.h \
	$(DRIVERS)/axi_core/jesd204/axi_adxcvr.h \
	$(DRIVERS)/axi_core/clk_axi_clkgen/clk_axi_clkgen.h
INCS +=	$(PLATFORM_DRIVERS)/$(PLATFORM)_spi.h \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_gpio.h
INCS +=	$(INCLUDE)/no_os_axi_io.h \
	$(INCLUDE)/no_os_spi.h \
	$(INCLUDE)/no_os_gpio.h \
	$(INCLUDE)/no_os_error.h \
	$(INCLUDE)/no_os_delay.h \
	$(INCLUDE)/no_os_util.h \
	$(INCLUDE)/no_os_clk.h \
	$(INCLUDE)/no_os_print_log.h \
	$(INCLUDE)/no_os_alloc.h \
	$(INCLUDE)/no_os_mutex.h \
	$(INCLUDE)/jesd204.h \
	$(NO-OS)/jesd204/jesd204-priv.h
