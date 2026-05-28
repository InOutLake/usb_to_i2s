#ifndef AUDIO_TRANSPORT_H
#define AUDIO_TRANSPORT_H

#include <stdint.h>
#include "stm32h7rsxx_hal.h"

#define AUDIO_SAMPLE_RATE_DEF   44100
#define AUDIO_CHANNELS          2
#define AUDIO_BYTES_PER_SAMPLE  4

#define AUDIO_PERIOD_SAMPLES    64 
#define AUDIO_DMA_BUF_SIZE      (AUDIO_PERIOD_SAMPLES * AUDIO_CHANNELS * 2)

#define USB_RING_BUFFER_SIZE    (AUDIO_PERIOD_SAMPLES * AUDIO_CHANNELS * 16)

void Audio_Transport_Init(void);
void Audio_Transport_Write_USB(const uint8_t* data, uint32_t size);
void Audio_Transport_Update_DMA(uint8_t half);

#endif
