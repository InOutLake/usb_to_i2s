#include "audio_transport.h"
#include "linked_list.h"
#include "main.h"             
#include "stm32h7rsxx_hal.h"

// Externs updated to match your exact CubeMX layout names
extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef handle_HPDMA1_Channel0;       // The actual DMA handle tied to SAI1-A
extern DMA_NodeTypeDef HPDMA_SAIA;          // The actual hardware node definition
extern DMA_QListTypeDef YourQueueName;      // Your Queue name from linked_list.c

// Both buffers remain securely locked in the non-cacheable section
__attribute__((section("noncacheable_buffer"))) uint32_t I2S_PingPong_Buffer[AUDIO_DMA_BUF_SIZE];
__attribute__((section("noncacheable_buffer"))) uint32_t USB_Ring_Buffer[USB_RING_BUFFER_SIZE];

static volatile uint32_t rb_head = 0;
static volatile uint32_t rb_tail = 0;

void Audio_Transport_Init(void) {
    for(int i = 0; i < AUDIO_DMA_BUF_SIZE; i++) I2S_PingPong_Buffer[i] = 0;
    for(int i = 0; i < USB_RING_BUFFER_SIZE; i++) USB_Ring_Buffer[i] = 0;

    // 1. Re-use the setup function CubeMX generated for you
    MX_YourQueueName_Config();

    // 2. Safely unpack and populate runtime addresses into our node configuration
    DMA_NodeConfTypeDef nodeConfig = {0};
    nodeConfig.SrcAddress      = (uint32_t)I2S_PingPong_Buffer;
    nodeConfig.DstAddress      = (uint32_t)&(SAI1_Block_A->DR);
    nodeConfig.DataSize        = sizeof(I2S_PingPong_Buffer);

    // 3. Rebuild the node to apply our custom runtime memory address modifications
    HAL_DMAEx_List_BuildNode(&nodeConfig, &HPDMA_SAIA);

    // 4. Link the complete queue structure directly to our live peripheral channel handle
    HAL_DMAEx_List_LinkQ(&handle_HPDMA1_Channel0, &YourQueueName);
    
    // 5. Start the underlying hardware loop transfer via the HAL SAI wrapper
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*)I2S_PingPong_Buffer, AUDIO_DMA_BUF_SIZE);
}

// ... Audio_Transport_Write_USB, Update_DMA, and callbacks remain exactly as they are

// Запись "сырых" данных из USB эндпоинта
void Audio_Transport_Write_USB(const uint8_t* data, uint32_t size) {
    uint32_t samples_count = size / sizeof(uint32_t);
    uint32_t *src = (uint32_t*)data;

    for (uint32_t i = 0; i < samples_count; i++) {
        uint32_t next_head = (rb_head + 1) % USB_RING_BUFFER_SIZE;
        if (next_head != rb_tail) { // Защита от Overflow
            USB_Ring_Buffer[rb_head] = src[i];
            rb_head = next_head;
        }
    }
}

// Потоковый прерывание-копировщик
void Audio_Transport_Update_DMA(uint8_t half) {
    uint32_t target_idx = (half == 0) ? 0 : (AUDIO_DMA_BUF_SIZE / 2);
    uint32_t samples_to_copy = AUDIO_DMA_BUF_SIZE / 2;

    for (uint32_t i = 0; i < samples_to_copy; i++) {
        if (rb_tail != rb_head) {
            I2S_PingPong_Buffer[target_idx + i] = USB_Ring_Buffer[rb_tail];
            rb_tail = (rb_tail + 1) % USB_RING_BUFFER_SIZE;
        } else {
            // Underflow: компьютер не успел налить данные, гоним тишину вместо треска
            I2S_PingPong_Buffer[target_idx + i] = 0;
        }
    }
}

// Переопределяем стандартные коллбеки HAL SAI
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    if (hsai == &hsai_BlockA1) {
        Audio_Transport_Update_DMA(0);
    }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    if (hsai == &hsai_BlockA1) {
        Audio_Transport_Update_DMA(1);
    }
}
