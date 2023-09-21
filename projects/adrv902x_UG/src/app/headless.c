/***************************************************************************//**
 *   @file   headless.c
 *   @brief  adrv902x main project file.
 *   @author Georgge Mois (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef XILINX_PLATFORM
#include "xil_cache.h"
#endif /* XILINX_PLATFORM */

#include "no_os_print_log.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_spi.h"

#include "axi_adc_core.h"
#include "axi_dac_core.h"
#include "axi_adxcvr.h"
#include "axi_dmac.h"

#include "parameters.h"
#include "xilinx_spi.h"
#include "xilinx_gpio.h"

#ifdef IIO_SUPPORT
#include "iio_app.h"
#include "iio_axi_adc.h"
#include "iio_axi_dac.h"
#include "xilinx_uart.h"
#endif

#include "adrv9025.h"
#include "jesd204_clk.h"

#include "ad9528.h"

static adi_adrv9025_SpiSettings_t spiSettings = {
	.msbFirst = 1,
	.enSpiStreaming = 0,
	.autoIncAddrUp = 1,
	.fourWireMode = 1,
	.cmosPadDrvStrength = ADI_ADRV9025_CMOSPAD_DRV_STRONG,
};

adi_adrv9025_SpiSettings_t *adrv9025_spi_settings_get(void)
{
	return &spiSettings;
}

int main(void)
{
	struct adrv9025_init_param adrv9025_init_par = { 0 };
	struct adi_adrv9025_Device adrv9025_device = { 0 };
	struct ad9528_channel_spec ad9528_channels[14];
	struct ad9528_platform_data ad9528_pdata;
	struct ad9528_init_param ad9528_param;
	struct adrv9025_rf_phy *phy;
	struct ad9528_dev* ad9528_device;
	int status;

	struct xil_spi_init_param sip_extra = {
#ifdef PLATFORM_MB
		.type = SPI_PL,
#else
		.type = SPI_PS,
#endif
		.flags = 0
	};

	/* Initialize SPI structures */
	struct no_os_spi_init_param ad9528_spi_param = {
		.device_id = SPI_DEVICE_ID,
		.max_speed_hz = 1000000u,
		.chip_select = AD9528_CS,
		.mode = NO_OS_SPI_MODE_0,
		.platform_ops = &xil_spi_ops,
		.extra = &sip_extra
	};

	struct xil_gpio_init_param xil_gpio_param = {
#ifdef PLATFORM_MB
		.type = GPIO_PL,
#else
		.type = GPIO_PS,
#endif
		.device_id = GPIO_DEVICE_ID,
	};

	struct no_os_gpio_init_param clkchip_gpio_init_param = {
		.number = AD9528_RESET_B,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_param
	};

	struct axi_dmac_init rx_dmac_init = {
		"rx_dmac",
		0x9c400000,
		IRQ_DISABLED
	};
	struct axi_dmac *rx_dmac;
	struct axi_dmac_init tx_dmac_init = {
		"tx_dmac",
		0x9c420000,
		IRQ_DISABLED
	};
	struct axi_dmac *tx_dmac;

	ad9528_param.spi_init = ad9528_spi_param;

	// Export no_os_clk_desc for each channel
	ad9528_param.export_no_os_clk = true;

	// ad9528 defaults
	ad9528_param.gpio_resetb = &clkchip_gpio_init_param;
	ad9528_param.pdata = &ad9528_pdata;
	ad9528_param.pdata->num_channels = 14;
	ad9528_param.pdata->channels = &ad9528_channels[0];
	ad9528_init(&ad9528_param);

	// channel 0
	ad9528_channels[0].channel_num = 0;
	ad9528_channels[0].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[0].divider_phase = 0;
	ad9528_channels[0].channel_divider = 5;
	ad9528_channels[0].signal_source = SOURCE_SYSREF_VCO;
	ad9528_channels[0].output_dis = 0;

	// channel 1
	ad9528_channels[1].channel_num = 1;
	ad9528_channels[1].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[1].divider_phase = 0;
	ad9528_channels[1].channel_divider = 5;
	ad9528_channels[1].signal_source = SOURCE_VCO;
	ad9528_channels[1].output_dis = 0;

	// channel 3
	ad9528_channels[3].channel_num = 3;
	ad9528_channels[3].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[3].divider_phase = 0;
	ad9528_channels[3].channel_divider = 5;
	ad9528_channels[3].signal_source = SOURCE_VCO;
	ad9528_channels[3].output_dis = 0;

	// channel 12
	ad9528_channels[12].channel_num = 12;
	ad9528_channels[12].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[12].divider_phase = 0;
	ad9528_channels[12].channel_divider = 5;
	ad9528_channels[12].signal_source = SOURCE_SYSREF_VCO;
	ad9528_channels[12].output_dis = 0;

	// channel 13
	ad9528_channels[13].channel_num = 13;
	ad9528_channels[13].driver_mode = DRIVER_MODE_LVDS;
	ad9528_channels[13].divider_phase = 0;
	ad9528_channels[13].channel_divider = 5;
	ad9528_channels[13].signal_source = SOURCE_VCO;
	ad9528_channels[13].output_dis = 0;

	// pllx settings
	ad9528_param.pdata->spi3wire = 0;
	ad9528_param.pdata->vcxo_freq = 122880000;
	ad9528_param.pdata->osc_in_diff_en = 0;
	ad9528_param.pdata->refa_en = 1;
	ad9528_param.pdata->refa_diff_rcv_en = 1;
	ad9528_param.pdata->refb_diff_rcv_en = 0;
	ad9528_param.pdata->osc_in_diff_en = 0;
	/* JESD */
	ad9528_param.pdata->jdev_desired_sysref_freq = 0; // ToDo
	/* PLL1 config */
	ad9528_param.pdata->pll1_feedback_div = 4;
	ad9528_param.pdata->pll1_charge_pump_current_nA = 5000;
	ad9528_param.pdata->ref_mode = REF_MODE_EXT_REF;
	/* PLL2 config */
	ad9528_param.pdata->pll2_charge_pump_current_nA = 805000;
	ad9528_param.pdata->pll2_vco_div_m1 = 3;
	ad9528_param.pdata->pll2_r1_div = 1;
	ad9528_param.pdata->pll2_ndiv_a_cnt = 3;
	ad9528_param.pdata->pll2_ndiv_b_cnt = 27;
	ad9528_param.pdata->pll2_n2_div = 10;
	/* SYSREF config */
	ad9528_param.pdata->sysref_src = SYSREF_SRC_INTERNAL;
	ad9528_param.pdata->sysref_k_div = 512;
	ad9528_param.pdata->sysref_pattern_mode = SYSREF_PATTERN_NSHOT;
	ad9528_param.pdata->sysref_nshot_mode = SYSREF_NSHOT_8_PULSES;
	ad9528_param.pdata->sysref_req_trigger_mode = SYSREF_LEVEL_HIGH;
	ad9528_param.pdata->sysref_req_en = false;
	ad9528_param.pdata->rpole2 = RPOLE2_900_OHM;
	ad9528_param.pdata->rzero = RZERO_1850_OHM;
	ad9528_param.pdata->cpole1 = CPOLE1_16_PF;
	ad9528_param.pdata->pll1_bypass_en = false;
	ad9528_param.pdata->pll2_bypass_en = false;
	ad9528_param.pdata->stat0_pin_func_sel = 1; /* PLL1 & PLL2 Locked */
	ad9528_param.pdata->stat1_pin_func_sel = 7; /* REFA Correct */

	status = ad9528_setup(&ad9528_device, ad9528_param);
	if (status != 0) {
		printf("error: ad9528_setup() failed\n");
		goto error;
	}

	struct axi_adc_init rx_adc_init = {
		.name = "rx_adc",
		.base = 0x84a00000
	};
	struct axi_dac_init tx_dac_init = {
		.name = "tx_dac",
		.base = 0x84a04000,
		.channels = NULL,
		.rate = 3
	};

	struct jesd204_tx_init tx_jesd_init = {
		.name = "tx_jesd",
		.base = 0x84a90000,
		.octets_per_frame = 4,
		.frames_per_multiframe = 32,
		.converters_per_device = 8,
		.converter_resolution = 16,
		.bits_per_sample = 16,
		.high_density = 1,
		.control_bits_per_sample = 0,// optional
		.subclass = 1,
		.device_clk_khz = 245760,
		.lane_clk_khz = 9830400
	};

	struct jesd204_rx_init rx_jesd_init = {
		.name = "rx_jesd",
		.base = 0x84aa0000,
		.octets_per_frame = 4,//DT
		.frames_per_multiframe = 32,//DT
		.subclass = 1,
		.device_clk_khz = 245760,
		.lane_clk_khz = 9830400
	};

	struct adxcvr_init tx_adxcvr_init = {
		.name = "tx_adxcvr",
		.base = 0x84a80000,
		.sys_clk_sel = ADXCVR_SYS_CLK_QPLL0,
		.out_clk_sel = ADXCVR_REFCLK,
		.lpm_enable = 0,
		.lane_rate_khz = 9830400, // ToDo
		.ref_rate_khz = 245760,
	};
	struct adxcvr *tx_adxcvr;

	struct adxcvr_init rx_adxcvr_init = {
		.name = "rx_adxcvr",
		.base = 0x84a60000,
		.sys_clk_sel = ADXCVR_SYS_CLK_CPLL,
		.out_clk_sel = ADXCVR_REFCLK,
		.lpm_enable = 1,
		.lane_rate_khz = 9830400,// ToDo
		.ref_rate_khz = 245760,
	};
	struct adxcvr *rx_adxcvr;

	status = adxcvr_init(&tx_adxcvr, &tx_adxcvr_init);
	if (status)
		return status;

	status = adxcvr_init(&rx_adxcvr, &rx_adxcvr_init);
	if (status)
		return status;

	struct jesd204_clk rx_jesd_clk;
	struct jesd204_clk tx_jesd_clk;

	struct axi_jesd204_rx *rx_jesd;
	struct axi_jesd204_tx *tx_jesd;

	rx_jesd_clk.xcvr = rx_adxcvr;
	tx_jesd_clk.xcvr = tx_adxcvr;

	rx_jesd_init.lane_clk.dev_desc = &rx_jesd_clk;
	rx_jesd_init.lane_clk.hw_ch_num = 0;
	rx_jesd_init.lane_clk.name = "jesd_rx";
	rx_jesd_init.lane_clk.platform_ops = &jesd204_clk_ops;

	tx_jesd_init.lane_clk.dev_desc = &tx_jesd_clk;
	tx_jesd_init.lane_clk.hw_ch_num = 0;
	tx_jesd_init.lane_clk.name = "jesd_tx";
	tx_jesd_init.lane_clk.platform_ops = &jesd204_clk_ops;

	status = axi_jesd204_tx_init_jesd_fsm(&tx_jesd, &tx_jesd_init);
	if (status)
		return status;

	status = axi_jesd204_rx_init_jesd_fsm(&rx_jesd, &rx_jesd_init);
	if (status)
		return status;

	adrv9025_init_par.adrv9025_device = &adrv9025_device;
	adrv9025_init_par.dev_clk = ad9528_device->clk_desc[1];

	status = adrv9025_init(&phy, &adrv9025_init_par);
	if (status) {
		pr_err("error: adrv9025_init() failed\n");
		goto error;
	}

	status = axi_dac_init(&phy->tx_dac, &tx_dac_init);
	if (status)
		return status;
	status = axi_adc_init(&phy->rx_adc, &rx_adc_init);
	if (status)
		return status;

	status = adrv9025_post_setup(phy);
	if (status){
		pr_err("error: adrv9025_post_setup() failed\n");
		goto error;
	}

	axi_dmac_init(&tx_dmac, &tx_dmac_init);
	axi_dmac_init(&rx_dmac, &rx_dmac_init);

	adi_common_LogLevelSet(&phy->madDevice->common,
				ADI_HAL_LOG_ALL);

	status = adi_adrv9025_HwOpen(phy->madDevice, adrv9025_spi_settings_get());
	if (status) {
		printf("ERR\n");
		//return adrv9025_dev_err(phy);
	}

	struct jesd204_topology *topology;
	struct jesd204_topology_dev devs[] = {
		{
			.jdev = ad9528_device->jdev,
			.link_ids = {DEFRAMER0_LINK_TX, FRAMER0_LINK_RX},
			.links_number = 2,
			.is_sysref_provider = true,
		},
		{
			.jdev = rx_jesd->jdev,
			.link_ids = {FRAMER0_LINK_RX},
			.links_number = 1,
		},
		{
			.jdev = tx_jesd->jdev,
			.link_ids = {DEFRAMER0_LINK_TX},
			.links_number = 1,
		},
		{
			.jdev = phy->jdev,
			.link_ids = {DEFRAMER0_LINK_TX, FRAMER0_LINK_RX},
			.links_number = 2,
			.is_top_device = true,
		},
	};

	jesd204_topology_init(&topology, devs,
				  sizeof(devs)/sizeof(*devs));

	jesd204_fsm_start(topology, JESD204_LINKS_ALL);

	axi_jesd204_tx_status_read(tx_jesd);
	axi_jesd204_rx_status_read(rx_jesd);

	printf("Bye \n");

	return 0;

error:
	adi_adrv9025_HwClose(phy->madDevice);
	axi_adc_remove(phy->rx_adc);
	axi_dac_remove(phy->tx_dac);
	return status;
}
