// Diego Lopes Ferreira (https://github.com/Diego-Lopes-Ferreira)
/* # ADC:
  - ADC dual mode DMA with double buffer (triggered by hardware)
  - (software access one part, dma access the other)
*/

#include <stdio.h>
#include <string.h>

#include "drivers_adc.h"
#include "drivers_clock.h"
#include "drivers_dma.h"
#include "drivers_gpio.h"
#include "drivers_timer.h"
#include "drivers_usart.h"
#include "stm32g4xx.h"

#define SAMPLES_PER_CYCLE 512
#define ADC_DMA_BUFFER_SIZE 1024

volatile uint8_t pin_state = 0;
volatile uint8_t should_start_timer_2 = 0;
volatile uint8_t should_read = 0;  // If all conversions of a cycle ended
volatile uint8_t pad = 0;          // Sets the buffer padding

volatile uint32_t adc12_buffer[ADC_DMA_BUFFER_SIZE];

int main() {
  clock_setup();

  uint16_t i = 0;
  char msg_buffer[20];
  // uint32_t adc12_buffer[2 * SAMPLES_PER_CYCLE];
  uint16_t adc1_read = 0;
  uint16_t adc2_read = 0;

  // GPIO (Green LED and Blue button)
  gpio_setup_pin(PA5, 0b01);    // 01: output / PA5=internal LED
  gpio_setup_pin(PC13, 0b00);   // 00: input / PC13=blue button
  gpio_enable_irq(PC13, 0, 1);  // Setup PC13 as Rising Edge ISR

  // LPUART 1
  gpio_setup_af(PA3, 12U);         // PA3 -> LPUART1_RX [STLINKV3E_VCP_TX]
  gpio_setup_af(PA2, 12U);         // PA2 -> LPUART1_TX [STLINKV3E_VCP_RX]
  usart_setup(LPUART1, 0x5C3B2U);  // 170Mhz / 115200

  // ADC1
  adc_startup(ADC1);
  adc_enable(ADC1);
  gpio_setup_pin(PA1, 0b11);  // PA1 -> channel_2 -> analog mode
  adc_config_regular_channel(ADC1, 2, 0b100, 1);
  adc_config_regular_sequence_length(ADC1, 1);

  // ADC2
  adc_startup(ADC2);
  adc_enable(ADC2);
  gpio_setup_pin(PA4, 0b11);  // PA4 -> channel_17 -> analog mode
  adc_config_regular_channel(ADC2, 17, 0b100, 1);
  adc_config_regular_sequence_length(ADC2, 1);

  // DMA stuff
  dma_channel_setup(DMA1_Channel1,                     // 1st channel
                    (uint32_t) & (ADC12_COMMON->CDR),  // source
                    (uint32_t) & (adc12_buffer),       // destination
                    ADC_DMA_BUFFER_SIZE);              // 10 measurements

  // Dual mode
  adc_dual_12_setup();

  // Start conversions
  adc_start_regular(ADC1);
  adc_start_regular(ADC2);

  // TIMER 2 (1s)
  // timer_setup(TIM2, 170, 100000);  // Timer (1ms)
  // timer_setup(TIM2, 2, 2767);      // Timer (256 samples per sec)
  timer_setup(TIM2, 4, 2767);      // Timer (512 samples per sec)

  memset(msg_buffer, 0, sizeof(msg_buffer));        // Reset buffer
  sprintf(msg_buffer, "Alive! \r\n");               // Write buffer
  usart_write(LPUART1, (uint8_t *)msg_buffer, 16);  // Send buffer

  while (1) {
    if (should_start_timer_2 == 1) {
      timer_start(TIM2);
      should_start_timer_2 = 0;
    }

    if (should_read == 0) {
      __asm__("NOP");
      continue;
    }
    should_read = 0;  // Clear flag

    for (i = 0; i < SAMPLES_PER_CYCLE; i++) {
      adc1_read = (adc12_buffer[i + pad] & 0xffff);
      adc2_read = ((adc12_buffer[i + pad] & 0xffff0000) >> 16);

      // Index Message
      memset(msg_buffer, 0, sizeof(msg_buffer));       // Reset buffer
      sprintf(msg_buffer, "%03d ", i);                 // Write buffer
      usart_write(LPUART1, (uint8_t *)msg_buffer, 4);  // Send buffer

      // ADC 1 Message
      memset(msg_buffer, 0, sizeof(msg_buffer));        // Reset buffer
      sprintf(msg_buffer, "ADC1: %04u  ", adc1_read);   // Write buffer
      usart_write(LPUART1, (uint8_t *)msg_buffer, 12);  // Send buffer

      // ADC 2 Message
      memset(msg_buffer, 0, sizeof(msg_buffer));          // Reset buffer
      sprintf(msg_buffer, "ADC2: %04u \r\n", adc2_read);  // Write buffer
      usart_write(LPUART1, (uint8_t *)msg_buffer, 13);    // Send buffer

    }  // for each of the ADC reads
  }
  return 0;
}

void EXTI15_10_IRQHandler() {
  EXTI->PR1 |= (1U << 14);  // because PC13
  if (should_start_timer_2 == 0) should_start_timer_2 = 1;
}

void DMA1_CH1_IRQHandler() {
  if (DMA1->ISR & (1U << 3)) {  // error
    DMA1->IFCR |= (1U << 3);    // Clear flag

  } else if (DMA1->ISR & (1U << 2)) {  // half transfer
    DMA1->IFCR |= (1U << 2);           // Clear flag
    timer_stop(TIM2);
    pad = 0;
    if (should_read == 0) should_read = 1;

  } else if (DMA1->ISR & (1U << 1)) {  // full transfer
    DMA1->IFCR |= (1U << 1);           // Clear flag
    timer_stop(TIM2);
    pad = 5;
    if (should_read == 0) should_read = 1;
  }
}

void DMAMUX_OVR_IRQHandler() { __asm__("NOP"); }
