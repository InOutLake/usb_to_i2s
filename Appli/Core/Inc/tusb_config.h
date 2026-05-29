#include "usb_descriptors.h"
#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define N_FREQUENCIES 6
//--------------------------------------------------------------------+
// BOARD TARGET & CONTROLLER CONFIGURATION
//--------------------------------------------------------------------+
#define CFG_TUSB_MCU OPT_MCU_STM32H7RS

// STM32H7 High-Speed controller is usually RHPORT 1 (OTG_HS)
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)
#define CFG_TUP_MCU_STMicroelectronics

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

// RHPort number used for device can be defined by board.mk, default to port 0
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT 0
#endif

// RHPort max operational speed can defined by board.mk
#ifndef BOARD_TUD_MAX_SPEED
#define BOARD_TUD_MAX_SPEED OPT_MODE_HIGH_SPEED
#endif

//--------------------------------------------------------------------
// Common Configuration
//--------------------------------------------------------------------

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG 0
#endif

// Enable Device stack
#define CFG_TUD_ENABLED 1

// Enable feedback loop
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP 1

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED BOARD_TUD_MAX_SPEED

/* USB DMA on some MCUs can only access a specific SRAM region with restriction
 * on alignment. Tinyusb use follows macros to declare transferring memory so
 * that they can be put into those specific section. e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

//------------- CLASS -------------//
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_HID 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_AUDIO 1
#define CFG_TUD_VENDOR 0

//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------

// Allow volume controlled by on-board button
#define CFG_TUD_AUDIO_ENABLE_INTERRUPT_EP 1

// How many formats are used, need to adjust USB descriptor if changed
#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS 2

// Audio format type I specifications
/* 24bit/48kHz is the best quality for headset or 24bit/96kHz for 2ch speaker,
   high-speed is needed beyond this */
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE 384000
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX 2

// Format 1 (e.g., 32-bit audio)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX 4
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX 32

// Format 2 (e.g., 24-bit audio fallback, 4 bytes container depending on
// standard)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_RX 4
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_RX 24

// -- CAPTURE / IN ENDPOINT DISABLED --
#define CFG_TUD_AUDIO_ENABLE_EP_IN 0

// -- PLAYBACK / OUT ENDPOINT ENABLED --
#define CFG_TUD_AUDIO_ENABLE_EP_OUT 1

// UAC1 (Full-Speed) Endpoint size calculation
#define CFG_TUD_AUDIO10_FUNC_1_FORMAT_1_EP_SZ_OUT                              \
  TUD_AUDIO_EP_SIZE(false, CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,               \
                    CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX,       \
                    CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)

// UAC2 (High-Speed) Endpoint size calculation
#define CFG_TUD_AUDIO20_FUNC_1_FORMAT_1_EP_SZ_OUT                              \
  TUD_AUDIO_EP_SIZE(true, CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                \
                    CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX,       \
                    CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)
#define CFG_TUD_AUDIO20_FUNC_1_FORMAT_2_EP_SZ_OUT                              \
  TUD_AUDIO_EP_SIZE(true, CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                \
                    CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_RX,       \
                    CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)

// Maximum EP OUT size for all AS alternate settings used
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX                                     \
  TU_MAX(CFG_TUD_AUDIO10_FUNC_1_FORMAT_1_EP_SZ_OUT,                            \
         TU_MAX(CFG_TUD_AUDIO20_FUNC_1_FORMAT_1_EP_SZ_OUT,                     \
                CFG_TUD_AUDIO20_FUNC_1_FORMAT_2_EP_SZ_OUT))

// Rx flow control needs buffer size >= 4* EP size to work correctly
// Example read FIFO every 1ms (8 HS frames), so buffer size should be 8 times
// larger for HS device
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ                                  \
  TU_MAX(4 * CFG_TUD_AUDIO10_FUNC_1_FORMAT_1_EP_SZ_OUT,                        \
         TU_MAX(32 * CFG_TUD_AUDIO20_FUNC_1_FORMAT_1_EP_SZ_OUT,                \
                32 * CFG_TUD_AUDIO20_FUNC_1_FORMAT_2_EP_SZ_OUT))

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
