/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ADRV9025
 *
 * Copyright 2020-2023 Analog Devices Inc.
 *
 * Licensed under the GPL-2.
 */

#ifndef IIO_TRX_ADRV9025_H_
#define IIO_TRX_ADRV9025_H_

#include "adi_adrv9025_agc.h"
#include "adi_adrv9025_arm.h"
#include "adi_adrv9025_cals.h"
#include "adi_adrv9025_dfe.h"
#include "adi_adrv9025_error.h"
#include "adi_adrv9025_gpio.h"
#include "adi_adrv9025_hal.h"
#include "adi_adrv9025_radioctrl.h"
#include "adi_adrv9025_rx.h"
#include "adi_adrv9025_tx.h"
#include "adi_adrv9025_user.h"
#include "adi_adrv9025_utilities.h"
#include "adi_adrv9025_version.h"
#include "adi_adrv9025_data_interface.h"
#include "adi_platform.h"

#include "no_os_mutex.h"
#include "no_os_clk.h"
#include <stdbool.h>
#include <stdint.h>

#define MIN_GAIN_mdB 0
#define MAX_RX_GAIN_mdB 30000
#define MAX_OBS_RX_GAIN_mdB 30000
#define RX_GAIN_STEP_mdB 500

 /*
  * JESD204-FSM defines
  */

#define DEFRAMER0_LINK_TX	0
#define DEFRAMER1_LINK_TX	1
#define FRAMER0_LINK_RX		2
#define FRAMER1_LINK_RX		3
#define FRAMER2_LINK_RX		4

enum debugfs_cmd {
	DBGFS_NONE,
	DBGFS_BIST_FRAMER_0_PRBS,
	DBGFS_BIST_FRAMER_LOOPBACK,
	DBGFS_BIST_TONE,
};

enum adrv9025_rx_ext_info {
	RSSI,
	RX_QEC,
	RX_HD2,
	RX_DIG_DC,
	RX_RF_BANDWIDTH,
};

enum adrv9025_tx_ext_info {
	TX_QEC,
	TX_LOL,
	TX_RF_BANDWIDTH,
};

enum adrv9025_iio_voltage_in {
	CHAN_RX1,
	CHAN_RX2,
	CHAN_RX3,
	CHAN_RX4,
	CHAN_OBS_RX1,
	CHAN_OBS_RX2,
	CHAN_OBS_RX3,
	CHAN_OBS_RX4,}
;

enum adrv9025_iio_voltage_out {
	CHAN_TX1,
	CHAN_TX2,
	CHAN_TX3,
	CHAN_TX4,
};

enum adrv9025_device_id {
	ID_ADRV9025,
	ID_ADRV9026,
	ID_ADRV9029,
};

struct adrv9025_rf_phy;
struct adrv9025_debugfs_entry {
	struct adrv9025_rf_phy *phy;
	const char *propname;
	void *out_value;
	uint32_t val;
	uint8_t size;
	uint8_t cmd;
};

enum adrv9025_clocks {
	RX_SAMPL_CLK,
	TX_SAMPL_CLK,
	NUM_ADRV9025_CLKS,
};

struct adrv9025_clock {
	struct no_os_clk_hw hw;
	struct spi_device *spi;
	struct adrv9025_rf_phy *phy;
	unsigned long rate;
	enum adrv9025_clocks source;
};

//#define to_clk_priv(_hw) container_of(_hw, struct adrv9025_clock, hw)
#define MAX_NUM_GAIN_TABLES 10

struct adrv9025_rf_phy {
	struct no_os_spi_desc		*spi_desc;
	adi_adrv9025_Device_t adi_adrv9025_device;
	adi_adrv9025_Device_t *madDevice;
	adi_adrv9025_SpiSettings_t spiSettings;
	adi_adrv9025_Init_t deviceInitStruct;
	adi_adrv9025_PlatformFiles_t platformFiles;
	adi_adrv9025_PostMcsInit_t adrv9025PostMcsInitInst;
	adi_adrv9025_InitCals_t cal_mask;

	struct jesd204_dev	*jdev;
	/* protect against device accesses */
	void				*lock;

	uint32_t tx_iqRate_kHz;
	uint32_t rx_iqRate_kHz;

	adi_hal_Cfg_t no_os_hal;
	struct no_os_clk *dev_clk;

	struct clk *clk_ext_lo_rx;
	struct clk *clk_ext_lo_tx;
	struct clk *clks[NUM_ADRV9025_CLKS];
	struct adrv9025_clock clk_priv[NUM_ADRV9025_CLKS];
//	struct clk_onecell_data clk_data;
//	struct adrv9025_debugfs_entry debugfs_entry[342];
//	struct iio_dev *indio_dev;

	struct no_os_gpio_desc *sysref_req_gpio;

	uint8_t device_id;

	uint32_t adrv9025_debugfs_entry_index;
	uint32_t tracking_cal_mask;

	bool is_initialized;
	int spi_device_id;
};

struct adrv9025_init_param {
	struct no_os_spi_init_param	*spi_init;
	struct no_os_gpio_init_param	*sysref_req_gpio_init;
	struct no_os_gpio_init_param	*reset_gpio_init;
//	struct no_os_clk	*dev_clk;
//	struct no_os_clk	*jesd_rx_clk;
//	struct no_os_clk	*jesd_tx_clk;
//	uint8_t		master_slave_sync_gpio_num;
//	bool		sysref_coupling_ac_en;
//	bool		sysref_cmos_input_enable;
//	uint8_t 	sysref_cmos_single_end_term_pos;
//	uint8_t 	sysref_cmos_single_end_term_neg;
//	uint32_t	multidevice_instance_count;
//	bool		jesd_sync_pins_01_swap_enable;
//	bool 		config_sync_0a_cmos_enable;
//	uint32_t	lmfc_delay_dac_clk_cycles;
//	uint32_t	nco_sync_ms_extra_lmfc_num;
//	bool		nco_sync_direct_sysref_mode_enable;
//	uint32_t	sysref_average_cnt_exp;
//	bool		continuous_sysref_mode_disable;
//	bool		tx_disable;
//	bool		rx_disable;
//	/* TX */
//	uint64_t	dac_frequency_hz;
//	/* The 4 DAC Main Datapaths */
//	uint32_t	tx_main_interpolation;
//	int64_t		tx_main_nco_frequency_shift_hz[MAX_NUM_MAIN_DATAPATHS];
//	uint8_t		tx_dac_channel_crossbar_select[MAX_NUM_MAIN_DATAPATHS];
//	uint8_t		tx_maindp_dac_1x_non1x_crossbar_select[MAX_NUM_MAIN_DATAPATHS];
//	uint32_t	tx_full_scale_current_ua[MAX_NUM_MAIN_DATAPATHS];
//	/* The 8 DAC Channelizers */
//	uint32_t	tx_channel_interpolation;
//	int64_t		tx_channel_nco_frequency_shift_hz[MAX_NUM_CHANNELIZER];
//	uint16_t	tx_channel_gain[MAX_NUM_CHANNELIZER];
//	struct link_init_param	*jrx_link_tx[2];
//	/* RX */
//	uint64_t 	adc_frequency_hz;
//	uint32_t	nyquist_zone[MAX_NUM_MAIN_DATAPATHS];
//	/* The 4 ADC Main Datapaths */
//	int64_t		rx_main_nco_frequency_shift_hz[MAX_NUM_MAIN_DATAPATHS];
//	uint32_t	rx_main_decimation[MAX_NUM_MAIN_DATAPATHS];
//	uint8_t		rx_main_complex_to_real_enable[MAX_NUM_MAIN_DATAPATHS];
//	uint8_t		rx_main_digital_gain_6db_enable[MAX_NUM_MAIN_DATAPATHS];
//	uint8_t		rx_main_enable[MAX_NUM_MAIN_DATAPATHS];
//	/* The 8 ADC Channelizers */
//	int64_t		rx_channel_nco_frequency_shift_hz[MAX_NUM_CHANNELIZER];
//	uint32_t	rx_channel_decimation[MAX_NUM_CHANNELIZER];
//	uint8_t		rx_channel_complex_to_real_enable[MAX_NUM_CHANNELIZER];
//	uint8_t		rx_channel_nco_mixer_mode[MAX_NUM_CHANNELIZER];
//	uint8_t		rx_channel_digital_gain_6db_enable[MAX_NUM_CHANNELIZER];
//	uint8_t		rx_channel_enable[MAX_NUM_CHANNELIZER];
//	uint8_t		rx_cddc_nco_channel_select_mode[MAX_NUM_MAIN_DATAPATHS];
//	uint8_t		rx_ffh_gpio_mux_selection[6];
//	struct link_init_param	*jtx_link_rx[2];
};

int adrv9025_hdl_loopback(struct adrv9025_rf_phy *phy, bool enable);
int adrv9025_register_axi_converter(struct adrv9025_rf_phy *phy);
struct adrv9025_rf_phy *adrv9025_spi_to_phy(struct spi_device *spi);
int adrv9025_spi_read(struct no_os_spi_desc *spi, uint32_t reg);
int adrv9025_spi_write(struct no_os_spi_desc *spi, uint32_t reg, uint32_t val);

int adrv9025_init(struct adrv9025_rf_phy **dev, const struct adrv9025_init_param *init_param);
int adrv9025_remove(struct adrv9025_rf_phy *phy);

#endif
