/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "audio_transport.h"
#include "common/tusb_common.h"
#include "tusb.h"
#include "usb_descriptors.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DMA_HandleTypeDef handle_HPDMA1_Channel0;

SAI_HandleTypeDef hsai_BlockA1;

PCD_HandleTypeDef hpcd_USB_OTG_HS;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_HPDMA1_Init(void);
static void MX_USB_OTG_HS_PCD_Init(void);
static void MX_SAI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Update SystemCoreClock variable according to RCC registers values. */
  SystemCoreClockUpdate();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */
  Audio_Transport_Init();
  tusb_init();

  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_HPDMA1_Init();
  MX_USB_OTG_HS_PCD_Init();
  MX_SAI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */
    tud_task();
    // audio_task();
    // audio_control_task();
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief HPDMA1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_HPDMA1_Init(void) {

  /* USER CODE BEGIN HPDMA1_Init 0 */

  /* USER CODE END HPDMA1_Init 0 */

  /* Peripheral clock enable */
  __HAL_RCC_HPDMA1_CLK_ENABLE();

  /* USER CODE BEGIN HPDMA1_Init 1 */

  /* USER CODE END HPDMA1_Init 1 */
  handle_HPDMA1_Channel0.Instance = HPDMA1_Channel0;
  handle_HPDMA1_Channel0.InitLinkedList.Priority = DMA_HIGH_PRIORITY;
  handle_HPDMA1_Channel0.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
  handle_HPDMA1_Channel0.InitLinkedList.LinkAllocatedPort =
      DMA_LINK_ALLOCATED_PORT0;
  handle_HPDMA1_Channel0.InitLinkedList.TransferEventMode =
      DMA_TCEM_LAST_LL_ITEM_TRANSFER;
  handle_HPDMA1_Channel0.InitLinkedList.LinkedListMode =
      DMA_LINKEDLIST_CIRCULAR;
  if (HAL_DMAEx_List_Init(&handle_HPDMA1_Channel0) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_DMA_ConfigChannelAttributes(&handle_HPDMA1_Channel0,
                                      DMA_CHANNEL_NPRIV) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN HPDMA1_Init 2 */

  /* USER CODE END HPDMA1_Init 2 */
}

/**
 * @brief SAI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SAI1_Init(void) {

  /* USER CODE BEGIN SAI1_Init 0 */

  /* USER CODE END SAI1_Init 0 */

  /* USER CODE BEGIN SAI1_Init 1 */

  /* USER CODE END SAI1_Init 1 */
  hsai_BlockA1.Instance = SAI1_Block_A;
  hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_TX;
  hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_192K;
  hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
  if (HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD,
                           SAI_PROTOCOL_DATASIZE_32BIT, 2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN SAI1_Init 2 */

  /* USER CODE END SAI1_Init 2 */
}

/**
 * @brief USB_OTG_HS Initialization Function
 * @param None
 * @retval None
 */
static void MX_USB_OTG_HS_PCD_Init(void) {

  /* USER CODE BEGIN USB_OTG_HS_PCD_Init 0 */

  /* USER CODE END USB_OTG_HS_PCD_Init 0 */

  /* USER CODE BEGIN USB_OTG_HS_PCD_Init 1 */

  /* USER CODE END USB_OTG_HS_PCD_Init 1 */
  hpcd_USB_OTG_HS.Instance = USB_OTG_HS;
  hpcd_USB_OTG_HS.Init.dev_endpoints = 9;
  hpcd_USB_OTG_HS.Init.speed = PCD_SPEED_HIGH;
  hpcd_USB_OTG_HS.Init.phy_itface = USB_OTG_HS_EMBEDDED_PHY;
  hpcd_USB_OTG_HS.Init.dma_enable = ENABLE;
  hpcd_USB_OTG_HS.Init.Sof_enable = DISABLE;
  hpcd_USB_OTG_HS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_HS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_HS.Init.use_dedicated_ep1 = DISABLE;
  hpcd_USB_OTG_HS.Init.vbus_sensing_enable = ENABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_HS) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_HS_PCD_Init 2 */

  /* USER CODE END USB_OTG_HS_PCD_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOM_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

static void MPU_Config(void) {
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /* Disables all MPU regions */
  for (uint8_t i = 0; i < __MPU_REGIONCOUNT; i++) {
    HAL_MPU_DisableRegion(i);
  }

  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_HFNMI_PRIVDEF);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
//--------------------------------------------------------------------+

// List of supported sample rates
const uint32_t sample_rates[] = {44100, 48000};

uint32_t current_sample_rate = 44100;

#define N_SAMPLE_RATES TU_ARRAY_SIZE(sample_rates)

/* Blink pattern
 * - 25 ms   : streaming data
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_STREAMING = 25,
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

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

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

// Audio controls
// Current states
uint8_t mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1]; // +1 for master channel 0
int16_t
    volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1]; // +1 for master channel 0

// Buffer for microphone data
int32_t mic_buf[CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ / 4];
// Buffer for speaker data
int32_t spk_buf[CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ / 4];
// Speaker data size received in the last frame
int spk_data_size;
// Resolution per format
const uint8_t resolutions_per_format[CFG_TUD_AUDIO_FUNC_1_N_FORMATS] = {
    CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX,
    CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_RX};
// Current resolution, update on format change
uint8_t current_resolution;

void led_blinking_task(void);
void audio_task(void);
void audio_control_task(void);

/*------------- MAIN -------------*/
// int main(void) {
//   board_init();
//
//   // init device stack on configured roothub port
//   tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE,
//                                  .speed = TUSB_SPEED_AUTO};
//   tusb_init(BOARD_TUD_RHPORT, &dev_init);
//
//   board_init_after_tusb();
//
//   TU_LOG1("Headset running\r\n");
//
//   while (1) {
//     tud_task(); // TinyUSB device task
//     audio_task();
//     audio_control_task();
//     led_blinking_task();
//   }
// }

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

void tud_mount_cb(void) { blink_interval_ms = BLINK_MOUNTED; }
void tud_umount_cb(void) { blink_interval_ms = BLINK_NOT_MOUNTED; }
void tud_suspend_cb(bool remote_wakeup_en) {
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// Audio Callback Functions
//--------------------------------------------------------------------+

//--------------------------------------------------------------------+
// UAC2 Helper Functions
//--------------------------------------------------------------------+

#if TUD_OPT_HIGH_SPEED

// Helper for clock get requests
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

// Helper for clock set requests
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

    TU_LOG1("Clock set current freq: %" PRIu32 "\r\n", current_sample_rate);

    return true;
  } else {
    TU_LOG1("Clock set request not supported, entity = %u, selector = %u, "
            "request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);
    return false;
  }
}

// Helper for feature unit get requests
static bool
audio20_feature_unit_get_request(uint8_t rhport,
                                 audio20_control_request_t const *request) {
  TU_ASSERT(request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT);

  if (request->bControlSelector == AUDIO20_FU_CTRL_MUTE &&
      request->bRequest == AUDIO20_CS_REQ_CUR) {
    audio20_control_cur_1_t mute1 = {.bCur = mute[request->bChannelNumber]};
    TU_LOG1("Get channel %u mute %d\r\n", request->bChannelNumber, mute1.bCur);
    return tud_audio_buffer_and_schedule_control_xfer(
        rhport, (tusb_control_request_t const *)request, &mute1, sizeof(mute1));
  } else if (request->bControlSelector == AUDIO20_FU_CTRL_VOLUME) {
    if (request->bRequest == AUDIO20_CS_REQ_RANGE) {
      audio20_control_range_2_n_t(1)
          range_vol = {.wNumSubRanges = tu_htole16(1),
                       .subrange[0] = {.bMin = tu_htole16(-VOLUME_CTRL_50_DB),
                                       tu_htole16(VOLUME_CTRL_0_DB),
                                       tu_htole16(256)}};
      TU_LOG1("Get channel %u volume range (%d, %d, %u) dB\r\n",
              request->bChannelNumber, range_vol.subrange[0].bMin / 256,
              range_vol.subrange[0].bMax / 256,
              range_vol.subrange[0].bRes / 256);
      return tud_audio_buffer_and_schedule_control_xfer(
          rhport, (tusb_control_request_t const *)request, &range_vol,
          sizeof(range_vol));
    } else if (request->bRequest == AUDIO20_CS_REQ_CUR) {
      audio20_control_cur_2_t cur_vol = {
          .bCur = tu_htole16(volume[request->bChannelNumber])};
      TU_LOG1("Get channel %u volume %d dB\r\n", request->bChannelNumber,
              cur_vol.bCur / 256);
      return tud_audio_buffer_and_schedule_control_xfer(
          rhport, (tusb_control_request_t const *)request, &cur_vol,
          sizeof(cur_vol));
    }
  }
  TU_LOG1("Feature unit get request not supported, entity = %u, selector = %u, "
          "request = %u\r\n",
          request->bEntityID, request->bControlSelector, request->bRequest);

  return false;
}

// Helper for feature unit set requests
static bool
audio20_feature_unit_set_request(uint8_t rhport,
                                 audio20_control_request_t const *request,
                                 uint8_t const *buf) {
  (void)rhport;

  TU_ASSERT(request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT);
  TU_VERIFY(request->bRequest == AUDIO20_CS_REQ_CUR);

  if (request->bControlSelector == AUDIO20_FU_CTRL_MUTE) {
    TU_VERIFY(request->wLength == sizeof(audio20_control_cur_1_t));

    mute[request->bChannelNumber] =
        ((audio20_control_cur_1_t const *)buf)->bCur;

    TU_LOG1("Set channel %d Mute: %d\r\n", request->bChannelNumber,
            mute[request->bChannelNumber]);

    return true;
  } else if (request->bControlSelector == AUDIO20_FU_CTRL_VOLUME) {
    TU_VERIFY(request->wLength == sizeof(audio20_control_cur_2_t));

    volume[request->bChannelNumber] =
        ((audio20_control_cur_2_t const *)buf)->bCur;

    TU_LOG1("Set channel %d volume: %d dB\r\n", request->bChannelNumber,
            volume[request->bChannelNumber] / 256);

    return true;
  } else {
    TU_LOG1("Feature unit set request not supported, entity = %u, selector = "
            "%u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);
    return false;
  }
}

static bool audio20_get_req_entity(uint8_t rhport,
                                   tusb_control_request_t const *p_request) {
  audio20_control_request_t const *request =
      (audio20_control_request_t const *)p_request;

  if (request->bEntityID == UAC2_ENTITY_CLOCK)
    return audio20_clock_get_request(rhport, request);
  if (request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT)
    return audio20_feature_unit_get_request(rhport, request);
  else {
    TU_LOG1(
        "Get request not handled, entity = %d, selector = %d, request = %d\r\n",
        request->bEntityID, request->bControlSelector, request->bRequest);
  }
  return false;
}

static bool audio20_set_req_entity(uint8_t rhport,
                                   tusb_control_request_t const *p_request,
                                   uint8_t *buf) {
  audio20_control_request_t const *request =
      (audio20_control_request_t const *)p_request;

  if (request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT)
    return audio20_feature_unit_set_request(rhport, request, buf);
  if (request->bEntityID == UAC2_ENTITY_CLOCK)
    return audio20_clock_set_request(rhport, request, buf);
  TU_LOG1(
      "Set request not handled, entity = %d, selector = %d, request = %d\r\n",
      request->bEntityID, request->bControlSelector, request->bRequest);

  return false;
}

#endif // TUD_OPT_HIGH_SPEED

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request,
                             uint8_t *pBuff) {
  (void)rhport;
  (void)pBuff;

  if (tud_audio_version() == 1) {
    return audio10_set_req_ep(p_request, pBuff);
  } else if (tud_audio_version() == 2) {
    // We do not support any requests here
  }

  return false; // Yet not implemented
}

// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request) {
  (void)rhport;

  if (tud_audio_version() == 1) {
    return audio10_get_req_ep(rhport, p_request);
  } else if (tud_audio_version() == 2) {
    // We do not support any requests here
  }

  return false; // Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request) {
  (void)rhport;

  if (tud_audio_version() == 1) {
    return audio10_get_req_entity(rhport, p_request);
#if TUD_OPT_HIGH_SPEED
  } else if (tud_audio_version() == 2) {
    return audio20_get_req_entity(rhport, p_request);
#endif
  }

  return false;
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request,
                                 uint8_t *buf) {
  (void)rhport;

  if (tud_audio_version() == 1) {
    return audio10_set_req_entity(p_request, buf);
#if TUD_OPT_HIGH_SPEED
  } else if (tud_audio_version() == 2) {
    return audio20_set_req_entity(rhport, p_request, buf);
#endif
  }

  return false;
}

bool tud_audio_set_itf_close_ep_cb(uint8_t rhport,
                                   tusb_control_request_t const *p_request) {
  (void)rhport;

  uint8_t const itf = tu_u16_low(p_request->wIndex);
  uint8_t const alt = tu_u16_low(p_request->wValue);

  if (ITF_NUM_AUDIO_STREAMING_SPK == itf && alt == 0) {
    blink_interval_ms = BLINK_MOUNTED;
  }

  return true;
}

bool tud_audio_set_itf_cb(uint8_t rhport,
                          tusb_control_request_t const *p_request) {
  (void)rhport;
  uint8_t const itf = tu_u16_low(p_request->wIndex);
  uint8_t const alt = tu_u16_low(p_request->wValue);

  TU_LOG2("Set interface %d alt %d\r\n", itf, alt);
  if (ITF_NUM_AUDIO_STREAMING_SPK == itf && alt != 0) {
    blink_interval_ms = BLINK_STREAMING;
  }

  // Clear buffer when streaming format is changed
  spk_data_size = 0;
  if (alt != 0) {
    current_resolution = resolutions_per_format[alt - 1];
  }

  return true;
}

//--------------------------------------------------------------------+
// AUDIO Task
//--------------------------------------------------------------------+

// This task simulates an audio transfer callback, one frame is sent/received
// every 1ms. In a real application, this would be replaced with actual I2S
// send/receive callback.
void audio_task(void) {
  static uint32_t start_ms = 0;
  uint32_t curr_ms = tusb_time_millis_api();
  if (start_ms == curr_ms)
    return; // not enough time
  start_ms = curr_ms;
  // When new data arrived, copy data from speaker buffer, to microphone buffer
  // and send it over
  // Only support speaker & headphone both have the same resolution
  // If one is 16bit another is 24bit be care of LOUD noise !
  spk_data_size = tud_audio_read(spk_buf, sizeof(spk_buf));
  if (spk_data_size) {
    if (current_resolution == 16) {
      int16_t *src = (int16_t *)spk_buf;
      int16_t *limit = (int16_t *)spk_buf + spk_data_size / 2;
      int16_t *dst = (int16_t *)mic_buf;
      while (src < limit) {
        // Combine two channels into one
        int32_t left = *src++;
        int32_t right = *src++;
        *dst++ = (int16_t)((left >> 1) + (right >> 1));
      }
      tud_audio_write((uint8_t *)mic_buf, (uint16_t)(spk_data_size / 2));
      spk_data_size = 0;
    } else if (current_resolution == 24) {
      int32_t *src = spk_buf;
      int32_t *limit = spk_buf + spk_data_size / 4;
      int32_t *dst = mic_buf;
      while (src < limit) {
        // Combine two channels into one
        int32_t left = *src++;
        int32_t right = *src++;
        *dst++ =
            (int32_t)((uint32_t)((left >> 1) + (right >> 1)) & 0xffffff00ul);
      }
      tud_audio_write((uint8_t *)mic_buf, (uint16_t)(spk_data_size / 2));
      spk_data_size = 0;
    }
  }
}

void audio_control_task(void) {
  // Press on-board button to control volume
  // Open host volume control, volume should switch between 10% and 100%

  // Poll every 50ms
  const uint32_t interval_ms = 50;
  static uint32_t start_ms = 0;
  static uint32_t btn_prev = 0;

  if (tusb_time_millis_api() - start_ms < interval_ms)
    return; // not enough time
  start_ms += interval_ms;

  uint32_t btn = board_button_read();

  // Even UAC1 spec have status interrupt support like UAC2, most host do not
  // support it So you have to either use UAC2 or use old day HID volume control
  TU_VERIFY((tud_audio_version() == 1), );

  if (!btn_prev && btn) {
    // Adjust volume between 0dB (100%) and -30dB (10%)
    for (int i = 0; i < CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1; i++) {
      volume[i] = volume[i] == 0 ? -VOLUME_CTRL_30_DB : 0;
    }

    // 6.1 Interrupt Data Message
    const audio_interrupt_data_t data = {
        .v2 = {
            .bInfo =
                0, // Class-specific interrupt, originated from an interface
            .bAttribute = AUDIO20_CS_REQ_CUR,    // Caused by current settings
            .wValue_cn_or_mcn = 0,               // CH0: master volume
            .wValue_cs = AUDIO20_FU_CTRL_VOLUME, // Volume change
            .wIndex_ep_or_int = 0,               // From the interface itself
            .wIndex_entity_id =
                UAC2_ENTITY_SPK_FEATURE_UNIT, // From feature unit
        }};

    tud_audio_int_write(&data);
  }

  btn_prev = btn;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if (tusb_time_millis_api() - start_ms < blink_interval_ms)
    return;
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state;
}
