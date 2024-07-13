#ifndef setup_h
#define setup_h

// #include "user_config/ESP32_ILI9341.h"
// #include "user_config/m5coreS3.h"
// #include "user_config/m5stack_basic.h"
// #include "user_config/rpipico_ILI9341_parallel.h"
// #include "user_config/rpipico_ILI9342_spi.h"
#include "user_config/rpipico_PicoResTouch_2_8.h"

#if defined(TP_XR_PIN_NO) && defined(TP_YD_PIN_NO) && defined(TP_XL_PIN_NO) && defined(TP_YU_PIN_NO)
#define USE_ANALOG_TOUCH_PANEL 1
#endif

#if defined(ARDUINO_ARCH_RP2040)
 #define ENABLE_MULTI_CORE 1
 #define ENABLE_TFT_DMA 1
 #define STORE_SINGLEBYTEGLYPH_TO_RAM 1
#elif defined(ARDUINO_ARCH_ESP32)
 #define ENABLE_TFT_DMA 1
 #define gpio_set_input_hysteresis_enabled(x,y) (NULL)
#else
 #define gpio_set_input_hysteresis_enabled(x,y) (NULL)
#endif

#endif // setup_h
