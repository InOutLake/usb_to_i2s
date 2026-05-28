#include "audio_transport.h"
#include "stm32h7xx_hal.h"
#include "tusb.h"

// Этот коллбек вызывается TinyUSB автоматически каждый раз,
// когда от ПК прилетает очередной ISO-пакет с аудиосэмплами (каждые 125 мкс)
void tud_audio_rx_done_cb(uint8_t rhport, uint8_t feature_unit_id,
                          uint8_t channel_num, uint16_t n_bytes) {
  (void)rhport;
  (void)feature_unit_id;
  (void)channel_num;

  // Временный стек-буфер для вычерпывания данных из USB-контроллера
  static uint8_t usb_rx_tmp[512];

  if (n_bytes > 0 && n_bytes <= sizeof(usb_rx_tmp)) {
    // Забираем данные из внутреннего буфера TinyUSB
    tud_audio_read(usb_rx_tmp, n_bytes);

    // Проталкиваем "сырые" байты в наш программный Ring Buffer в ОЗУ D2
    Audio_Transport_Write_USB(usb_rx_tmp, n_bytes);
  }
}

// Perfect match for your header signature
bool tud_audio_set_itf_cb(uint8_t rhport,
                          tusb_control_request_t const *p_request) {
  (void)rhport;

  uint32_t sample_rate = tud_audio_get_sample_rate();

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;

  if ((sample_rate % 48000) == 0) {
    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
  } else if ((sample_rate % 44100) == 0) {
    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL3;
  } else {
    return false;
  }

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    return false;
  }

  return true;
}
