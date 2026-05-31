#include "tusb.h"
#include "main.h"
#include "audio_device.h"
#include "stm32h7rsxx.h"
#include "stm32h7rsxx_hal_gpio.h"
#include "stm32h7rsxx_hal_rcc_ex.h"
#include "usb_callbacks.h"

// Ring buffer trackers
#define BUFFER_SIZE_SAMPLES (2048)

uint32_t current_sample_rate = 44100;

uint32_t const sample_rates[] = {44100, 48000, 88200, 96000, 176400, 192000};
#define N_SAMPLE_RATES (sizeof(sample_rates) / sizeof(sample_rates[0]))

int32_t spk_buf[CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ / 4];
int spk_data_size;
const uint8_t resolutions_per_format[CFG_TUD_AUDIO_FUNC_1_N_FORMATS] = {CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX,
                                                                        CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_RX};
uint8_t current_resolution;


RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

/* ----- Clock controls ----- */
static bool
audio20_clock_get_request(uint8_t rhport,
                          audio20_control_request_t const *request) {
  TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);

  if (request->bControlSelector == AUDIO20_CS_CTRL_SAM_FREQ) {
    if (request->bRequest == AUDIO20_CS_REQ_CUR) {
      TU_LOG1("Clock get current freq %" PRIu32 "\r\n", current_sample_rate);

      audio20_control_cur_4_t curf = {(int32_t)tu_htole32(current_sample_rate)};
      return tud_audio_buffer_and_schedule_control_xfer(
          rhport, (tusb_control_request_t const *)request, &curf, sizeof(curf));
    } else if (request->bRequest == AUDIO20_CS_REQ_RANGE) {
      audio20_control_range_4_n_t(N_SAMPLE_RATES)
          rangef = {.wNumSubRanges = tu_htole16(N_SAMPLE_RATES)};
      TU_LOG1("Clock get %d freq ranges\r\n", N_SAMPLE_RATES);
      for (uint8_t i = 0; i < N_SAMPLE_RATES; i++) {
        rangef.subrange[i].bMin = (int32_t)sample_rates[i];
        rangef.subrange[i].bMax = (int32_t)sample_rates[i];
        rangef.subrange[i].bRes = 0;
        TU_LOG1("Range %d (%d, %d, %d)\r\n", i, (int)rangef.subrange[i].bMin,
                (int)rangef.subrange[i].bMax, (int)rangef.subrange[i].bRes);
      }

      return tud_audio_buffer_and_schedule_control_xfer(
          rhport, (tusb_control_request_t const *)request, &rangef,
          sizeof(rangef));
    }
  } else if (request->bControlSelector == AUDIO20_CS_CTRL_CLK_VALID &&
             request->bRequest == AUDIO20_CS_REQ_CUR) {
    audio20_control_cur_1_t cur_valid = {.bCur = 1};
    TU_LOG1("Clock get is valid %u\r\n", cur_valid.bCur);
    return tud_audio_buffer_and_schedule_control_xfer(
        rhport, (tusb_control_request_t const *)request, &cur_valid,
        sizeof(cur_valid));
  }
  TU_LOG1("Clock get request not supported, entity = %u, selector = %u, "
          "request = %u\r\n",
          request->bEntityID, request->bControlSelector, request->bRequest);
  return false;
}

static bool audio20_clock_set_request(uint8_t rhport,
                                      audio20_control_request_t const *request,
                                      uint8_t const *buf) {
  (void)rhport;

  TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);
  TU_VERIFY(request->bRequest == AUDIO20_CS_REQ_CUR);

  if (request->bControlSelector == AUDIO20_CS_CTRL_SAM_FREQ) {
    TU_VERIFY(request->wLength == sizeof(audio20_control_cur_4_t));

    current_sample_rate =
        (uint32_t)((audio20_control_cur_4_t const *)buf)->bCur;

    if (current_sample_rate / 48000 == 0) {
      PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
      PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2P;
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 0);
    } else {
      PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
      PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL3P;
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 1);
    }

    TU_LOG1("Clock set current freq: %" PRIu32 "\r\n", current_sample_rate);

    return true;
  } else {
    TU_LOG1("Clock set request not supported, entity = %u, selector = %u, "
            "request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);
    return false;
  }
}

/* ----- Audio controls ----- */
// TODO: rsearch audio controls, it may not be necessary or work improperly on the board I have.

uint8_t mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1]; // targeting master channel +1 
int16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1];
enum {
  VOLUME_CTRL_0_DB = 0,
  VOLUME_CTRL_10_DB = 2560,
  VOLUME_CTRL_20_DB = 5120,
  VOLUME_CTRL_30_DB = 7680,
  VOLUME_CTRL_40_DB = 10240,
  VOLUME_CTRL_50_DB = 12800,
  VOLUME_CTRL_60_DB = 15360,
  VOLUME_CTRL_70_DB = 17920,
  VOLUME_CTRL_80_DB = 20480,
  VOLUME_CTRL_90_DB = 23040,
  VOLUME_CTRL_100_DB = 25600,
  VOLUME_CTRL_SILENCE = 0x8000,
};
static bool audio20_feature_unit_get_request(uint8_t rhport, audio20_control_request_t const *request) {
  TU_ASSERT(request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT);

  if (request->bControlSelector == AUDIO20_FU_CTRL_MUTE && request->bRequest == AUDIO20_CS_REQ_CUR) {
    audio20_control_cur_1_t mute1 = {.bCur = mute[request->bChannelNumber]};
    TU_LOG1("Get channel %u mute %d\r\n", request->bChannelNumber, mute1.bCur);
    return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *) request, &mute1, sizeof(mute1));
  } else if (request->bControlSelector == AUDIO20_FU_CTRL_VOLUME) {
    if (request->bRequest == AUDIO20_CS_REQ_RANGE) {
      audio20_control_range_2_n_t(1) range_vol = {
          .wNumSubRanges = tu_htole16(1),
          .subrange[0] = {.bMin = tu_htole16(-VOLUME_CTRL_50_DB), tu_htole16(VOLUME_CTRL_0_DB), tu_htole16(256)}};
      TU_LOG1("Get channel %u volume range (%d, %d, %u) dB\r\n", request->bChannelNumber,
              range_vol.subrange[0].bMin / 256, range_vol.subrange[0].bMax / 256, range_vol.subrange[0].bRes / 256);
      return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *) request, &range_vol, sizeof(range_vol));
    } else if (request->bRequest == AUDIO20_CS_REQ_CUR) {
      audio20_control_cur_2_t cur_vol = {.bCur = tu_htole16(volume[request->bChannelNumber])};
      TU_LOG1("Get channel %u volume %d dB\r\n", request->bChannelNumber, cur_vol.bCur / 256);
      return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *) request, &cur_vol, sizeof(cur_vol));
    }
  }
  TU_LOG1("Feature unit get request not supported, entity = %u, selector = %u, request = %u\r\n",
          request->bEntityID, request->bControlSelector, request->bRequest);

  return false;
}

static bool audio20_feature_unit_set_request(uint8_t rhport, audio20_control_request_t const *request, uint8_t const *buf) {
  (void) rhport;

  TU_ASSERT(request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT);
  TU_VERIFY(request->bRequest == AUDIO20_CS_REQ_CUR);

  if (request->bControlSelector == AUDIO20_FU_CTRL_MUTE) {
    TU_VERIFY(request->wLength == sizeof(audio20_control_cur_1_t));

    mute[request->bChannelNumber] = ((audio20_control_cur_1_t const *) buf)->bCur;

    TU_LOG1("Set channel %d Mute: %d\r\n", request->bChannelNumber, mute[request->bChannelNumber]);

    return true;
  } else if (request->bControlSelector == AUDIO20_FU_CTRL_VOLUME) {
    TU_VERIFY(request->wLength == sizeof(audio20_control_cur_2_t));

    volume[request->bChannelNumber] = ((audio20_control_cur_2_t const *) buf)->bCur;

    TU_LOG1("Set channel %d volume: %d dB\r\n", request->bChannelNumber, volume[request->bChannelNumber] / 256);

    return true;
  } else {
    TU_LOG1("Feature unit set request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);
    return false;
  }
}

// Works on button, not necessary
// void audio_control_task(void) {
//   const uint32_t interval_ms = 50;
//   static uint32_t start_ms = 0;
//   static uint32_t btn_prev = 0;
//
//   if (tusb_time_millis_api() - start_ms < interval_ms) return;// not enough time
//   start_ms += interval_ms;
//
//
//   // Even UAC1 spec have status interrupt support like UAC2, most host do not support it
//   // So you have to either use UAC2 or use old day HID volume control
//   TU_VERIFY((tud_audio_version() == 1),);
//
//   if (!btn_prev && btn) {
//     // Adjust volume between 0dB (100%) and -30dB (10%)
//     for (int i = 0; i < CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1; i++) {
//       volume[i] = volume[i] == 0 ? -VOLUME_CTRL_30_DB : 0;
//     }
//
//     // 6.1 Interrupt Data Message
//     const audio_interrupt_data_t data = {.v2 = {
//         .bInfo = 0,                                      // Class-specific interrupt, originated from an interface
//         .bAttribute = AUDIO20_CS_REQ_CUR,                // Caused by current settings
//         .wValue_cn_or_mcn = 0,                           // CH0: master volume
//         .wValue_cs = AUDIO20_FU_CTRL_VOLUME,             // Volume change
//         .wIndex_ep_or_int = 0,                           // From the interface itself
//         .wIndex_entity_id = UAC2_ENTITY_SPK_FEATURE_UNIT,// From feature unit
//     }};
//
//     tud_audio_int_write(&data);
//   }
//
//   btn_prev = btn;
// }

/* ----- Firmware control ----- */
static bool audio20_get_req_entity(uint8_t rhport, tusb_control_request_t const *p_request) {
  audio20_control_request_t const *request = (audio20_control_request_t const *) p_request;

  if (request->bEntityID == UAC2_ENTITY_CLOCK)
    return audio20_clock_get_request(rhport, request);
  if (request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT)
    return audio20_feature_unit_get_request(rhport, request);
  else {
    TU_LOG1("Get request not handled, entity = %d, selector = %d, request = %d\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);
  }
  return false;
}

static bool audio20_set_req_entity(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *buf) {
  audio20_control_request_t const *request = (audio20_control_request_t const *) p_request;

  if (request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT)
    return audio20_feature_unit_set_request(rhport, request, buf);
  if (request->bEntityID == UAC2_ENTITY_CLOCK)
    return audio20_clock_set_request(rhport, request, buf);
  TU_LOG1("Set request not handled, entity = %d, selector = %d, request = %d\r\n",
          request->bEntityID, request->bControlSelector, request->bRequest);

  return false;
}

/* ----- Blinking ----- */

uint32_t tusb_time_millis(void) 
{
  return HAL_GetTick();
}

enum {
  BLINK_STREAMING = 25,
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
}

void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

void led_blinking_task(void) {
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if (tusb_time_millis() - start_ms < blink_interval_ms) return;
  start_ms += blink_interval_ms;

  HAL_GPIO_WritePin(SIGNAL_LED_GPIO_Port, SIGNAL_LED_Pin, led_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
  led_state = !led_state;
}


/* ----- Audio ----- */

TU_ATTR_FAST_FUNC void tud_audio_feedback_interval_isr(uint8_t func_id, uint32_t frame_number, uint8_t interval_shift)
{
  (void) frame_number;
  (void) interval_shift;

  uint32_t current_fill = (wr_ptr >= rd_ptr) ? (wr_ptr - rd_ptr) : (BUFFER_SIZE_SAMPLES - rd_ptr + wr_ptr);
  uint32_t target_fill = BUFFER_SIZE_SAMPLES / 2;

  // At 192kHz High-Speed, nominal is 24.0 samples per microframe.
  // 24 in 16.16 format is (24 << 16) = 1572864.
  uint32_t feedback_val = (current_sample_rate / 8000) << 16;

  int32_t error = (int32_t)current_fill - (int32_t)target_fill;
  
  feedback_val -= (error * 8); 

  tud_audio_n_fb_set(func_id, feedback_val);
}

void tud_audio_feedback_params_cb(uint8_t func_id, uint8_t alt_itf, audio_feedback_params_t* feedback_param)
{
  (void) func_id;
  (void) alt_itf;
  
  // manual fifo tracking
  feedback_param->method = AUDIO_FEEDBACK_METHOD_DISABLED; 
}

__attribute__((section(".d2_noncacheable"))) int32_t audio_buffer[BUFFER_SIZE_SAMPLES];

volatile uint32_t wr_ptr = 0;
volatile uint32_t rd_ptr = 0;

static uint8_t local_packet_chunk[CFG_TUD_AUDIO_FUNC_EP_OUT_SIZE_MAX];

void audio_task(void)
{
  if ( !tud_audio_mounted() )
  {
    return;
  }

  uint16_t available_bytes = tud_audio_available();

  if (available_bytes > 0)
  {
    if (available_bytes > sizeof(local_packet_chunk))
    {
      available_bytes = sizeof(local_packet_chunk);
    }

    uint32_t bytes_read = tud_audio_read(local_packet_chunk, available_bytes);
    uint32_t samples_received = bytes_read / 4;

    int32_t *samples = (int32_t*)local_packet_chunk;

    for (uint32_t i = 0; i < samples_received; i++)
    {
      audio_buffer[wr_ptr] = samples[i];
      wr_ptr = (wr_ptr + 1) % BUFFER_SIZE_SAMPLES;
    }
  }
}
