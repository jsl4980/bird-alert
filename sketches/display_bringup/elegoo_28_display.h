#pragma once

/**
 * ELEGOO 2.8" / ESP32-2432S028-class SPI display — board profile "configuration 1".
 *
 * TFT:  ILI9341 on HSPI  GPIO 14/13/12  (CS 15, DC 2, BL 21)
 * Touch: XPT2046 on VSPI GPIO 25/32/39 (CS 33, IRQ 36) — NOT the TFT SPI lines.
 *
 * Touch alignment: use calibrateTouch() + NVS (see elegoo28_touch_cal.h).
 * Do not use offset_rotation / INVERT_Y hacks with stored cal[8].
 *
 * See docs/hardware.md and docs/display-bringup.md
 */

#include <Arduino.h>

// ---------------------------------------------------------------------------
// TFT SPI (HSPI / SPI2 on classic ESP32)
// ---------------------------------------------------------------------------
#define ELEGOO28_TFT_SCK 14
#define ELEGOO28_TFT_MOSI 13
#define ELEGOO28_TFT_MISO 12

#define ELEGOO28_TFT_DC 2
#define ELEGOO28_TFT_CS 15
#define ELEGOO28_TFT_RST (-1)
#define ELEGOO28_TFT_BL 21

// ---------------------------------------------------------------------------
// Touch SPI (VSPI / SPI3) — separate bus from TFT on integrated boards
// ---------------------------------------------------------------------------
#define ELEGOO28_TOUCH_SCK 25
#define ELEGOO28_TOUCH_MOSI 32
#define ELEGOO28_TOUCH_MISO 39
#define ELEGOO28_TOUCH_CS 33
#define ELEGOO28_TOUCH_IRQ 36

#if defined(CONFIG_IDF_TARGET_ESP32)
#if defined(HSPI_HOST) && defined(VSPI_HOST)
#define ELEGOO28_TFT_SPI_HOST HSPI_HOST
#define ELEGOO28_TOUCH_SPI_HOST VSPI_HOST
#elif defined(SPI2_HOST) && defined(SPI3_HOST)
#define ELEGOO28_TFT_SPI_HOST SPI2_HOST
#define ELEGOO28_TOUCH_SPI_HOST SPI3_HOST
#else
#define ELEGOO28_TFT_SPI_HOST 2
#define ELEGOO28_TOUCH_SPI_HOST 3
#endif
#else
#define ELEGOO28_TFT_SPI_HOST 2
#define ELEGOO28_TOUCH_SPI_HOST 3
#endif

// ---------------------------------------------------------------------------
// Display orientation
// ---------------------------------------------------------------------------
/** LovyanGFX setRotation() — re-calibrate in NVS if you change this. */
#define ELEGOO28_DISPLAY_ROTATION 1

/** Neutral touch mapping; corner calibrateTouch() supplies the real transform. */
#define ELEGOO28_TOUCH_OFFSET_ROTATION 0

// ---------------------------------------------------------------------------
// Touch calibration (NVS + LovyanGFX cal[8])
// ---------------------------------------------------------------------------
/** If no valid NVS cal exists, run interactive corner wizard on boot. */
#define ELEGOO28_TOUCH_REQUIRE_CALIBRATION 1

/** Set to 1 for one upload to force re-calibration (then set back to 0). */
#define ELEGOO28_TOUCH_FORCE_CALIBRATE 0

/** 1 = print raw XPT2046 on Serial while polling (debug only). */
#define ELEGOO28_TOUCH_DEBUG_RAW 0

/** Corner validation UI once after a fresh wizard (not on NVS reload). */
#define ELEGOO28_TOUCH_RUN_CORNER_TEST 1

/** Max |dx|/|dy| from expected corner during validation (pixels). */
#define ELEGOO28_TOUCH_CORNER_TOLERANCE 15

#define ELEGOO28_DISPLAY_NAME "ELEGOO 2.8\" ILI9341 + XPT2046 cfg1"

static inline void elegoo28_display_backlight_on(void) {
#if ELEGOO28_TFT_BL >= 0
  pinMode(ELEGOO28_TFT_BL, OUTPUT);
  digitalWrite(ELEGOO28_TFT_BL, HIGH);
#endif
}
