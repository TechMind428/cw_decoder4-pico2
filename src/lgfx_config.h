// LovyanGFX configuration for Raspberry Pi Pico 2 + ST7789 240x320.
// Touch is intentionally not configured; all operations use push switches.
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "board_config.h"

class LGFX : public lgfx::LGFX_Device {
	lgfx::Panel_ST7789 _panel_instance;
	lgfx::Bus_SPI _bus_instance;

public:
	LGFX(void) {
		{
			auto cfg = _bus_instance.config();
			cfg.spi_host = 0;
			cfg.spi_mode = 3;
			cfg.freq_write = 40000000;
			cfg.freq_read = 12000000;
			cfg.pin_sclk = PIN_LCD_SCK;
			cfg.pin_mosi = PIN_LCD_MOSI;
			cfg.pin_miso = -1;
			cfg.pin_dc = PIN_LCD_DC;
			_bus_instance.config(cfg);
			_panel_instance.setBus(&_bus_instance);
		}
		{
			auto cfg = _panel_instance.config();
			cfg.pin_cs = PIN_LCD_CS;
			cfg.pin_rst = PIN_LCD_RST;
			cfg.pin_busy = -1;
			cfg.memory_width = 240;
			cfg.memory_height = 320;
			cfg.panel_width = 240;
			cfg.panel_height = 320;
			cfg.offset_x = 0;
			cfg.offset_y = 0;
			cfg.offset_rotation = 0;
			cfg.invert = false;
			_panel_instance.config(cfg);
		}
		setPanel(&_panel_instance);
	}
};
