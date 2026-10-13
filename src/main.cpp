///////////////////////////////////////////////////////////////////////////
//
//	CW Decoder for Raspberry Pi Pico 2 (RP2350)
//	バージョン: 2.1
//
//	- Arduino-Pico + LovyanGFX (ST7789 240x320, 横向き 320x240)
//	- 音声入力: GP26 / ADC0 を連結DMAで8kHzサンプリング
//	- 画面: 上2/3 デコード文字 (16列x6行, 24x24全角フォント/カタカナ対応)
//	        下1/3 左FFT / 右オシロ (生波形+エンベロープ+キー判定を同期表示)
//	- 操作: 5個のプッシュスイッチ (タッチパネルなし)
//
//	原作: Hjalmar Skovholm Hansen OZ1JHM (GPL)
//	このソフトウェアは GNU General Public License (GPL) に基づき配布されています。
//
///////////////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include "audio.h"
#include "dsp.h"
#include "decoder.h"
#include "display.h"
#include "board_config.h"

#define FRAME_MS 33
#define DEBOUNCE_MS 30

// Arduino-Picoのsetup1()/loop1()用にCore 1へ独立8KBスタックを割り当てる。
bool core1_separate_stack = true;
static uint8_t system_ready = 0;

typedef struct {
	uint8_t pin;
	uint8_t raw;
	uint8_t stable;
	uint32_t changed_ms;
} button_t;

static button_t buttons[] = {
	{ PIN_SW_MODE,      HIGH, HIGH, 0 },
	{ PIN_SW_TONE_NEXT, HIGH, HIGH, 0 },
	{ PIN_SW_KEY,       HIGH, HIGH, 0 },
	{ PIN_SW_ENV,       HIGH, HIGH, 0 },
	{ PIN_SW_RAW,       HIGH, HIGH, 0 },
};

static void tone_next(void)
{
	dsp_set_tone((uint8_t)((dsp_tone_index() + 1) %
	                       (DSP_TONE_COUNT + 1)));
}

static void button_short_action(uint8_t index)
{
	switch (index) {
	case 0: decoder_toggle_mode(); break;
	case 1: tone_next(); break;
	case 2: display_toggle_key(); break;
	case 3: display_toggle_env(); break;
	case 4: display_toggle_raw(); break;
	}
}

static void poll_buttons(void)
{
	uint32_t now = millis();
	for (uint8_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
		button_t *b = &buttons[i];
		uint8_t raw = (uint8_t)digitalRead(b->pin);
		if (raw != b->raw) {
			b->raw = raw;
			b->changed_ms = now;
		}
		if (raw != b->stable && (now - b->changed_ms) >= DEBOUNCE_MS) {
			b->stable = raw;
			if (raw == HIGH) {
				button_short_action(i);
			}
		}
	}
}

void setup()
{
	Serial.begin(115200);
	for (uint8_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
		pinMode(buttons[i].pin, INPUT_PULLUP);
	}
	pinMode(PIN_CW_LED, OUTPUT);
	pinMode(PIN_TEST_OUT, OUTPUT);
	digitalWrite(PIN_CW_LED, LOW);
	digitalWrite(PIN_TEST_OUT, LOW);

	display_init();
	display_splash();

	decoder_init();
	decoder_set_emit(display_enqueue);

	__atomic_store_n(&system_ready, 1, __ATOMIC_RELEASE);
}

// Core 1: ADCとDSPを表示から分離して連続処理する。
void setup1()
{
	while (!__atomic_load_n(&system_ready, __ATOMIC_ACQUIRE)) delay(1);
	dsp_start();
	audio_init();
}

void loop1()
{
	dsp_update();
}

void loop()
{
	uint32_t t0 = millis();
	poll_buttons();
	display_update();
	uint32_t dt = millis() - t0;
	if (dt < FRAME_MS) {
		delay(FRAME_MS - dt);
	}
}
