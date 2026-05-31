#ifndef USB_CALLBACKS_H
#define USB_CALLBACKS_H
#include <stdint.h>

void audio_task(void);
// void audio_control_task(void);
void led_blinking_task(void);

#define BUFFER_SIZE_SAMPLES  (2048)
extern int32_t audio_buffer[BUFFER_SIZE_SAMPLES];
extern volatile uint32_t wr_ptr;
extern volatile uint32_t rd_ptr;


#endif // USB_CALLBACKS_H
