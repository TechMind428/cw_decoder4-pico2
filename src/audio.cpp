// Raspberry Pi Pico 2 ADC0 (GP26) capture using two chained DMA channels.
#include <Arduino.h>
#include <hardware/adc.h>
#include <hardware/dma.h>
#include <string.h>
#include "audio.h"
#include "dsp.h"
#include "board_config.h"

static int dma_chan[2] = { -1, -1 };
static uint16_t dma_buf[2][DSP_HOP];
static uint8_t read_buf = 0;

static void arm_channel(uint8_t idx)
{
	dma_channel_set_write_addr(dma_chan[idx], dma_buf[idx], false);
	dma_channel_set_trans_count(dma_chan[idx], DSP_HOP, false);
}

void audio_init(void)
{
	adc_init();
	adc_gpio_init(PIN_AUDIO_ADC);
	adc_select_input(0);
	adc_fifo_setup(true, true, 1, false, false);
	adc_set_clkdiv(48000000.0f / (float)DSP_SAMPLE_RATE - 1.0f);

	dma_chan[0] = dma_claim_unused_channel(true);
	dma_chan[1] = dma_claim_unused_channel(true);
	for (uint8_t i = 0; i < 2; ++i) {
		dma_channel_config cfg = dma_channel_get_default_config(dma_chan[i]);
		channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
		channel_config_set_read_increment(&cfg, false);
		channel_config_set_write_increment(&cfg, true);
		channel_config_set_dreq(&cfg, DREQ_ADC);
		channel_config_set_chain_to(&cfg, dma_chan[i ^ 1]);
		dma_channel_configure(dma_chan[i], &cfg, dma_buf[i], &adc_hw->fifo,
		                      DSP_HOP, false);
	}

	read_buf = 0;
	dma_start_channel_mask(1u << dma_chan[0]);
	adc_run(true);
}

size_t audio_read(uint16_t *dst, size_t n)
{
	if (n > DSP_HOP) n = DSP_HOP;
	const uint8_t idx = read_buf;
	dma_channel_wait_for_finish_blocking(dma_chan[idx]);
	memcpy(dst, dma_buf[idx], n * sizeof(uint16_t));
	arm_channel(idx); // re-arm before the other 6 ms buffer completes
	read_buf ^= 1;
	return n;
}
