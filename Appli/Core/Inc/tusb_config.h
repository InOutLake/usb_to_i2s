#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+
#define CFG_TUSB_MCU OPT_MCU_STM32H7RS
#define CFG_TUSB_RHPORT0_MODE                                                  \
  (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED) // Force High-Speed mode
#define CFG_TUSB_RHPORT1_MODE 0

#define CFG_TUD_AUDIO_FUNC_1_DESC_STANDARD TUD_AUDIO_DESC_TYPE_AUDIO_2_0

//--------------------------------------------------------------------
// Common Configuration
//--------------------------------------------------------------------

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG 0
#endif

#define CFG_TUD_ENABLED 1
#define CFG_TUD_MAX_SPEED                                                      \
  BOARD_TUD_MAX_SPEED // Ensure board init triggers TUSB_SPEED_HIGH
//
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_RX 4
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_RX 32

/* * STM32H7RS RAM Alignment for USB DMA.
 * Ensure your linker places USB buffers into a DMA-accessible SRAM section
 * (like AXISRAM).
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION __attribute__((section(".usb_ram")))
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

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
// AUDIO CLASS DRIVER CONFIGURATION (UAC2 32-bit / 192kHz)
//--------------------------------------------------------------------

#define CFG_TUD_AUDIO_ENABLE_INTERRUPT_EP 1
#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS 1

#define BOARD_TUD_MAX_SPEED TUSB_SPEED_HIGH

// Crucial: Update sample rate boundary
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE 192000

// Channels: 2 Channels Out (RX / Speaker), 0 Channels In (TX / Mic)
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX 0
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX 2

// Audio Format 1 Specification (32-bit slot / 32-bit audio resolution)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX 4
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX 32

// Endpoint Controls
#define CFG_TUD_AUDIO_ENABLE_EP_IN 0 // Disabled since TX channels = 0
#define CFG_TUD_AUDIO_ENABLE_EP_OUT 2

// UAC2 (High-Speed) Endpoint size calculation
// For 192000Hz, 2 channels, 4 bytes/sample -> 192000 * 2 * 4 = 1,536,000
// bytes/sec In High Speed (8000 microframes/sec), packet base size is ~192
// bytes.
#define CFG_TUD_AUDIO20_FUNC_1_FORMAT_1_EP_SZ_OUT                              \
  TUD_AUDIO_EP_SIZE(true, CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                \
                    CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX,       \
                    CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)

#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX                                     \
  CFG_TUD_AUDIO20_FUNC_1_FORMAT_1_EP_SZ_OUT

// High-speed ring buffers benefit from extra elasticity to prevent underflows
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ                                  \
  (16 * CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX)

#ifdef __cplusplus
}
#endif

#endif /* TUSB_CONFIG_H_ */
