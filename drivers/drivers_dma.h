/*
# DMA Drivers
## How to use
- Enable DMA (1 or 2 for STM32G474)
- Setup the DMA channel with equivalent DMAMUX
- Enable DMA interrupts (NVIC calls)
- Start DMA
*/
#include "stm32g4xx.h"

void dma_mux_enable();
void DMA1_enable();
void DMA2_enable();

void dma_channel_start(DMA_Channel_TypeDef *channel);
void dma_channel_stop(DMA_Channel_TypeDef *channel);

void dma_channel_setup(DMA_Channel_TypeDef *channel,         // DMA_Channel
                       DMAMUX_Channel_TypeDef *mux_channel,  // DMAMUX channel
                       uint8_t mux_request_input,            // description
                       uint32_t per_addr,                    // Peripheral address
                       uint8_t per_size,                     // Peripheral size
                       uint8_t per_increment,                // Peripheral increment?
                       uint32_t mem_addr,                    // Memory address
                       uint8_t mem_size,                     // Memory size (00: 8 bits, 01: 16 bits, 10: 32 bits)
                       uint8_t mem_increment,                // Memory increment?
                       uint16_t n_of_transfers,              // Number of transfers
                       uint8_t priority,                     // DMA_Channel priority
                       uint8_t direction,                    // Data direction (0: read from per, 1: read from mem)
                       uint8_t circular_mode,                // Circular mode?
                       uint8_t interrupts);                  // 0bXYZ = X: error, Y: Half, Z = Full
