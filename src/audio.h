// Audio capture (Raspberry Pi Pico 2 ADC0 via chained DMA)
#pragma once
#include <stdint.h>
#include <stddef.h>

void audio_init(void);
// Reads up to DSP_HOP samples (blocking). Returns 12-bit values (0..4095).
size_t audio_read(uint16_t *dst, size_t n);
