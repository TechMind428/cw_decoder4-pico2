//
// Raspberry Pi Pico 2 wiring (see CW_配線図のコピー.pptx)
//
#pragma once

// ST7789 / SPI0
#define PIN_LCD_DC       16
#define PIN_LCD_CS       17
#define PIN_LCD_SCK      18
#define PIN_LCD_MOSI     19
#define PIN_LCD_RST      20

// Audio and diagnostics
#define PIN_AUDIO_ADC    26  // ADC0
#define PIN_CW_LED       25
#define PIN_TEST_OUT     12

// Push switches (active LOW, internal pull-up)
#define PIN_SW_MODE      14  // US / JP
#define PIN_SW_TONE_NEXT 15
#define PIN_SW_KEY       13
#define PIN_SW_ENV       10
#define PIN_SW_RAW       11
