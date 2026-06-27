#pragma once

/**
 * LovyanGFX device: ILI9341 (HSPI) + XPT2046 touch (VSPI, separate pins).
 * Pin defines: elegoo_28_display.h
 */

#include <LovyanGFX.hpp>
#include "elegoo_28_display.h"

class LGFX_Elegoo28 : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Touch_XPT2046 _touch_instance;

 public:
  LGFX_Elegoo28(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = ELEGOO28_TFT_SPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = ELEGOO28_TFT_SCK;
      cfg.pin_mosi = ELEGOO28_TFT_MOSI;
      cfg.pin_miso = ELEGOO28_TFT_MISO;
      cfg.pin_dc = ELEGOO28_TFT_DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = ELEGOO28_TFT_CS;
      cfg.pin_rst = ELEGOO28_TFT_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 300;
      cfg.x_max = 3900;
      cfg.y_min = 400;
      cfg.y_max = 3900;
      cfg.pin_int = ELEGOO28_TOUCH_IRQ;
      cfg.bus_shared = false;
      cfg.offset_rotation = ELEGOO28_TOUCH_OFFSET_ROTATION;
      cfg.spi_host = ELEGOO28_TOUCH_SPI_HOST;
      cfg.freq = 2500000;
      cfg.pin_sclk = ELEGOO28_TOUCH_SCK;
      cfg.pin_mosi = ELEGOO28_TOUCH_MOSI;
      cfg.pin_miso = ELEGOO28_TOUCH_MISO;
      cfg.pin_cs = ELEGOO28_TOUCH_CS;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};
