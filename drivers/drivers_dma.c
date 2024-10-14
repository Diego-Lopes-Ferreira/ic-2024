#include "drivers_dma.h"

void dma_mux_enable() {
  RCC->AHB1ENR |= (1U << 2);  // Enable DMAMUX 1
  __DSB();
}

void DMA1_enable() {
  RCC->AHB1ENR |= (1U);  // Enable DMA 1
  __DSB();
}

void DMA2_enable() {
  RCC->AHB1ENR |= (1U << 1);  // Enable DMA 2
  __DSB();
}

void dma_channel_start(DMA_Channel_TypeDef *channel) {
  channel->CCR |= (1U);  // Enable
}

void dma_channel_stop(DMA_Channel_TypeDef *channel) {
  channel->CCR &= ~(1U);  // Enable
}

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
                       uint8_t interrupts) {                 // 0bXYZ = X: error, Y: Half, Z = Full
  dma_channel_stop(channel);

  // MEMORY
  channel->CMAR = mem_addr;
  if (mem_increment) channel->CCR |= (1U << 7);  // Memory increment
  channel->CCR |= (mem_size << 10);              // Memory size

  // PERIPHERAL
  channel->CPAR = per_addr;
  if (per_increment) channel->CCR |= (1U << 6);  // Peripheral increment
  channel->CCR |= (per_size << 8);               // Peripheral size

  // DATA DIRECTION
  if (direction == 0)
    channel->CCR &= ~(1U << 4);  // Read from peripheral
  else
    channel->CCR |= (1U << 4);  // Read from memory

  // CIRCULAR MODE, PRIORITY, NUMBER OF TRANSFERS and INTERRUPTS
  if (circular_mode == 1) channel->CCR |= (1U << 5);  // Circular mode
  channel->CCR |= (priority << 12);                   // Channel priority from 00 to 11
  channel->CNDTR = n_of_transfers;                    // Number of transfers
  // channel->CCR |= (interrupts << 1);                  // DMA interrupts: 0bXYZ = X: error, Y: Half, Z = Full
  if ((interrupts & 0b100) > 0) channel->CCR |= (1U << 3);  // Error
  if ((interrupts & 0b010) > 0) channel->CCR |= (1U << 2);  // Half
  if ((interrupts & 0b001) > 0) channel->CCR |= (1U << 1);  // Full

  // DMA MUX REQUEST INPUT NUMBER
  mux_channel->CCR |= mux_request_input;  // 5 is ADC1
}

/*
void dma_channel_setup(DMA_Channel_TypeDef *channel,         // DMA1_Channelx
                          DMAMUX_Channel_TypeDef *mux_channel,  // DMAMUX channel
                          uint32_t per_addr,                    // Peripheral address
                          uint32_t mem_addr,                    // Memory address
                          uint16_t n_of_transfers) {            // Number of transfers
  // Enable DMA1 and setup
  RCC->AHB1ENR |= (1U);       // Enable DMA 1
  RCC->AHB1ENR |= (1U << 2);  // Enable DMA 1
  __DSB();

  // Address and number of transfers
  channel->CPAR = per_addr;
  channel->CMAR = mem_addr;
  channel->CNDTR = n_of_transfers;

  // Channel priority and data direction
  channel->CCR |= (0b01 << 12);  // Channel priority from 00 to 11
  channel->CCR &= ~(1U << 4);    // Read from peripheral

  // Address configuration
  channel->CCR |= (1U << 5);  // Circular mode
  channel->CCR |= (1U << 7);  // Memory increment
  // channel->CCR |= (1U << 6);  // Peripheral increment

  // Data size
  channel->CCR |= (0b10 << 10);  // Memory size 32 bits
  channel->CCR |= (0b10 << 8);   // Peripheral size 32 bits

  // Interrupts
  channel->CCR |= (1U << 3);  // Error
  channel->CCR |= (1U << 2);  // Half
  channel->CCR |= (1U << 1);  // Full

  // TODO: Remove this (should be user handled)
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  NVIC_SetPriority(DMA1_Channel1_IRQn, 2);
  mux_channel->CCR |= (5U);  // 5 is ADC1

  channel->CCR |= (1U);  // Enable
}
*/
