#include "no_os_delay.h"
#include "no_os_print_log.h"
#include "no_os_util.h"
#include "hmc630x.h"
#include "parameters.h"

// select between admv9611 (high-band tx) and admv9621 (low-band tx)
const bool admv9611 = false;
const uint64_t txfreq = admv9611 ? 63000000000 : 58012500000;
const uint64_t rxfreq = admv9611 ? 58012500000 : 63000000000;

int main()
{
	int ret;
	struct no_os_uart_desc *uart_desc;
	struct no_os_gpio_desc *reset;

	ret = no_os_uart_init(&uart_desc, &uart_console_ip);
	if (ret)
		return ret;

	no_os_uart_stdio(uart_desc);

	struct hmc630x_dev *tx;
	struct hmc630x_init_param txip = {0};
	txip.type = HMC6300;
	txip.ref_clk = HMC6300_REF_CLK_75MHz;
	txip.en = en_gpio_ip;
	txip.clk = clk_gpio_ip;
	txip.data = data_gpio_ip;
	txip.scanout = scanout_tx_gpio_ip;

	struct hmc630x_dev *rx;
	struct hmc630x_init_param rxip = {0};
	rxip.type = HMC6301;
	rxip.ref_clk = HMC6300_REF_CLK_75MHz;
	rxip.en = en_gpio_ip;
	rxip.clk = clk_gpio_ip;
	rxip.data = data_gpio_ip;
	rxip.scanout = scanout_rx_gpio_ip;

	// initialize reset gpio separately
	ret = no_os_gpio_get(&reset, &reset_gpio_ip);
	if (ret)
		goto end;

	// set gpio direction
	ret = no_os_gpio_direction_output(reset, NO_OS_GPIO_LOW);
	if (ret)
		goto end;

	// reset
	no_os_gpio_set_value(reset, NO_OS_GPIO_HIGH);
	no_os_mdelay(1);
	no_os_gpio_set_value(reset, NO_OS_GPIO_LOW);
	no_os_mdelay(1);

	/* ----------------- HMC6300 ------------------ */
	printf(" ----- TX -----\n");
	ret = hmc630x_init(&tx, &txip);
	if (ret) {
		printf("Failed to initialize tx.\n");
		goto end;
	}

	// Enable
	ret = hmc630x_set_enable(tx, true);
	if (ret)
		goto end;

	// clear PA_PWRDWN_FAST (is this needed ? default is 1)
	ret = hmc630x_write(tx, HMC6300_PA_PWRDWN_FAST, 0);
	if (ret)
		goto end;

	// Enable temperature sensor
	ret = hmc630x_set_temp_en(tx, true);
	if (ret)
		goto end;

	// set IF attenuation
	ret = hmc630x_set_if_attn(tx, 13);
	if (ret)
		goto end;

	// set RF attenuation
	ret = hmc6300_set_rf_attn(tx, 15);
	if (ret)
		goto end;

	const uint64_t *freqs;
	uint8_t nfreqs;
	int n;
	ret = hmc630x_get_avail_vco(tx, &freqs, &nfreqs);
	if (ret)
		goto end;
	printf("Available frequencies: ");
	for (n = 0; n < nfreqs; n++)
		printf("%llu ", freqs[n]);
	printf("\n");

	// set VCO frequency
	ret = hmc630x_set_vco(tx, txfreq);
	if (ret)
		goto end;

	printf("Set VCO to %llu\n", txfreq);

	// print the whole register map
	int r;
	uint8_t regmap[32];
	ret = hmc630x_read_regmap(tx, regmap);
	if (ret)
		goto end;
	for (r = 0; r < 28; r++) {
		printf("Row %d: 0x%x (%d)\n", r, regmap[r], regmap[r]);
	}

	// read the temperature
	uint8_t temp;
	ret = hmc630x_get_temp(tx, &temp);
	if (ret)
		goto end;
	printf("Temperature: 0x%x\n", temp);

	/* ----------------- HMC6301 ------------------ */
	printf(" ----- RX -----\n");
	ret = hmc630x_init(&rx, &rxip);
	if (ret) {
		printf("Failed to initialize rx.\n");
		goto end;
	}

	ret = hmc630x_get_avail_vco(rx, &freqs, &nfreqs);
	if (ret)
		goto end;
	printf("Available frequencies: ");
	for (n = 0; n < nfreqs; n++)
		printf("%llu ", freqs[n]);
	printf("\n");

	// Enable temperature sensor
	ret = hmc630x_set_temp_en(rx, true);
	if (ret)
		goto end;

	// set IF attenuation
	ret = hmc630x_set_if_attn(rx, 11);
	if (ret)
		goto end;

	ret = hmc6301_set_lna_gain(rx, HMC6301_LNA_GAIN_12dB);
	if (ret)
		goto end;

	// Enable
	ret = hmc630x_set_enable(rx, true);
	if (ret)
		goto end;

	ret = hmc6301_set_bb_attn(rx, HMC6301_BB_ATTN_18dB, HMC6301_BB_ATTN_18dB);
	if (ret)
		goto end;

	ret = hmc6301_set_bb_attn_fine(rx, HMC6301_BB_ATTN_FINE_3dB, HMC6301_BB_ATTN_FINE_0dB);
	if (ret)
		goto end;

	ret = hmc6301_set_bb_lpc_hpc(rx, HMC6301_BB_LPC_1400MHz, HMC6301_BB_HPC_45kHz);
	if (ret)
		goto end;

	// set VCO frequency
	ret = hmc630x_set_vco(rx, rxfreq);
	if (ret)
		goto end;

	printf("Set VCO to %llu\n", rxfreq);

	// print the whole register map
	ret = hmc630x_read_regmap(rx, regmap);
	if (ret)
		goto end;
	for (r = 0; r < 28; r++) {
		printf("Row %d: 0x%x (%d)\n", r, regmap[r], regmap[r]);
	}

	// read the temperature
	ret = hmc630x_get_temp(rx, &temp);
	if (ret)
		goto end;
	printf("Temperature: 0x%x\n", temp);

end:
	// disable them
	hmc630x_set_enable(tx, false);
	hmc630x_set_enable(rx, false);

	hmc630x_remove(tx);
	hmc630x_remove(rx);

	printf("%s returned with %d\n", __func__, ret);
	return ret;
}
