#include "tusb.h"

// Ring buffer trackers
#define BUFFER_SIZE_SAMPLES (2048)
int32_t audio_buffer[BUFFER_SIZE_SAMPLES];
volatile uint32_t wr_ptr = 0;
volatile uint32_t rd_ptr = 0;

uint32_t current_sample_rate = 44100;

uint32_t const sample_rates[] = {44100, 48000, 88200, 96000, 176400, 192000};
#define N_SAMPLE_RATES (sizeof(sample_rates) / sizeof(sample_rates[0]))

// Set frequency request
bool tud_audio_set_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request,
                             uint8_t *p_buff) {
  (void)rhport;
  if (p_request->bRequest == AUDIO20_CS_REQ_CUR) {
    current_sample_rate = *((uint32_t const *)p_buff);
    return true;
  }
  return false;
}

bool tud_audio_get_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request) {
  (void)rhport;
  if (p_request->bRequest == AUDIO20_CS_REQ_CUR) {
    return tud_control_xfer(rhport, p_request, &current_sample_rate,
                            sizeof(current_sample_rate));
  }
  return false;
}

bool tud_audio_get_req_IT_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request) {
  (void)rhport;
  (void)p_request;
  return false;
}

bool tud_audio_get_req_CLK_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request) {
  (void)rhport;
  if (p_request->bRequest == AUDIO_CS_REQ_CUR) {
    return tud_control_xfer(rhport, p_request, &current_sample_rate,
                            sizeof(current_sample_rate));
  } else if (p_request->bRequest == AUDIO_CS_REQ_RANGE) {
    // Array layout for ranges: NumSubRanges (uint16_t), then Min, Max, Res
    // triplets.
    static uint8_t range_desc[2 + 3 * 4 * N_SAMPLE_RATES];
    uint16_t num_ranges = N_SAMPLE_RATES;
    memcpy(range_desc, &num_ranges, 2);

    for (size_t i = 0; i < N_SAMPLE_RATES; i++) {
      uint32_t rate = sample_rates[i];
      memcpy(&range_desc[2 + i * 12], &rate, 4);     // Min
      memcpy(&range_desc[2 + i * 12 + 4], &rate, 4); // Max
      uint32_t res = 1;
      memcpy(&range_desc[2 + i * 12 + 8], &res, 4); // Resolution
    }
    return tud_control_xfer(rhport, p_request, range_desc, sizeof(range_desc));
  }
  return false;
}

void tud_audio_feedback_params_cb(uint8_t func_id, uint8_t alt_itf,
                                  audio_feedback_params_t *feedback_param) {
  (void)func_id;
  (void)alt_itf;

  // Set feedback method to FIFO counting
  // This automatically attempts to keep the buffer at 50% capacity
  feedback_param->method = AUDIO_FEEDBACK_METHOD_FIFO_COUNT;

  // Set to your target sample rate
  feedback_param->sample_freq = current_sample_rate;
}

// 2. Read data from async buffer (TinyUSB FIFO -> ring buffer)
bool tud_audio_rx_done_pre_read_cb(uint8_t rhport, uint16_t n_bytes,
                                   uint8_t func_id, uint8_t ep_out,
                                   uint8_t alt_setting) {
  (void)rhport;
  (void)func_id;
  (void)ep_out;
  (void)alt_setting;

  uint8_t packet_buf[CFG_TUD_AUDIO_FUNC_EP_OUT_SIZE_MAX];
  uint32_t bytes_read = tud_audio_read(packet_buf, n_bytes);
  uint32_t samples_received = bytes_read / 4; // 32-bit samples

  int32_t *samples = (int32_t *)packet_buf;
  for (uint32_t i = 0; i < samples_received; i++) {
    audio_buffer[wr_ptr] = samples[i];
    wr_ptr = (wr_ptr + 1) % BUFFER_SIZE_SAMPLES;
  }
  return true;
}

// 3. Center the buffer (Explicit Feedback Generation)
// This is called automatically by the stack every SOF/uSOF frame to tell the
// host PC how to adjust its clock.
void tud_audio_fb_done_cb(uint8_t rhport) {
  (void)rhport;

  // Calculate buffer fill occupancy
  uint32_t current_fill = (wr_ptr >= rd_ptr)
                              ? (wr_ptr - rd_ptr)
                              : (BUFFER_SIZE_SAMPLES - rd_ptr + wr_ptr);
  uint32_t target_fill = BUFFER_SIZE_SAMPLES / 2;

  // Base feedback formula for High-Speed (16.16 format representing samples
  per
      // microframe) At 192kHz, 192000 / 8000 microframes = 24 samples per
      // microframe base.
      uint32_t feedback_val = (current_sample_rate / 8000) << 16;

  // Proportional correction to center the buffer
  int32_t error = (int32_t)current_fill - (int32_t)target_fill;

  // Adjust host output rate slightly based on buffer error
  feedback_val -= (error * 10);

  // Push feedback back to PC host
  tud_audio_fb_write(feedback_val);
}
