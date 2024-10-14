#include "drivers_adc.h"

void adc_startup(ADC_TypeDef *adc) {
  // Exits deep-power-down mode, starts voltage regulator and does calibration process
  volatile uint16_t i = 0;

  if ((adc == ADC1) || (adc == ADC2)) {
    RCC->CCIPR |= (0b01 << 28);  // PLLP as ADC clock
    __DSB();
    RCC->AHB2ENR |= (1U << 13);  // Enable ADC 12
  } else {
    RCC->CCIPR |= (0b01 << 30);  // PLLP as ADC clock
    __DSB();
    RCC->AHB2ENR |= (1U << 14);  // Enable ADC 345
  }
  // Wait 4 clock cycles
  while (i < 4) {
    i++;
    __DSB();
  }
  i = 0;

  adc->CR &= ~(1U << 29);  // Exit deep-power-down
  // Wait 20us (170MHz * 20us = 3400)
  while (i < 4000) {
    i++;
    __DSB();
  }
  i = 0;

  adc->CR |= (1U << 28);  // Enable voltage regulator
  // Wait 20us (170MHz * 20us = 3400)
  while (i < 4000) {
    i++;
    __DSB();
  }
  i = 0;

  adc->CR &= ~(1U << 30);                       // Select calibration for single ended
  adc->CR |= (1U << 31);                        // Start calibration
  while (adc->CR & (1U << 31)) __asm__("NOP");  // Wait for calibration

  // Wait 4 cycles (Software procedure to enable the ADC)
  while (i < 4) {
    i++;
    __DSB();
  }
}

void adc_enable(ADC_TypeDef *adc) {
  // Clear ADRDY flag, enable ADC and wait for startup
  adc->ISR |= 1U;                            // Clear ADRDY flag
  adc->CR |= 1U;                             // Enable ADC
  while (!(adc->ISR & 0b1)) __asm__("NOP");  // Wait for startup
}

void adc_disable(ADC_TypeDef *adc) {
  // Check if there is nothing running (disable if needed) set ADDIS and wait for ADEN=0
  adc_stop_injected(adc);
  adc_stop_regular(adc);
  while (adc->CR & (0b11 << 2)) __asm__("NOP");  // Wait for conversions to stop

  adc->ISR |= 1U;                        // Clear ADRDY flag
  adc->CR |= (1U << 1);                  // Disable ADC
  while (adc->CR & 0b1) __asm__("NOP");  // Wait for ADEN=0
}

void adc_start_regular(ADC_TypeDef *adc) {
  // Enable ADC->CR.ADSTART
  adc->CR |= (1U << 2);  // start regular conversions
}

void adc_stop_regular(ADC_TypeDef *adc) {
  // Enable ADC->CR.ADSTP
  if (adc->CR & (1U << 2)) {
    adc->CR |= (1U << 4);  // stop regular conversions
  }
}

void adc_start_injected(ADC_TypeDef *adc) {
  // Enable ADC->CR.JADSTRT
  adc->CR |= (1U << 3);  // start injected conversions
}

void adc_stop_injected(ADC_TypeDef *adc) {
  // Enable ADC->CR.JADSTP
  if (adc->CR & (1U << 3)) {
    adc->CR |= (1U << 5);  // stop injected conversions
  }
}

void adc_config_sample_time(ADC_TypeDef *adc, uint8_t channel, uint8_t sample_time) {
  // Update ADC->SMPRx with "sample_time" value on "channel" position
  // NOTE(dd): DOES NOT CHECK FOR ADSTART or JADSTART
  if (channel < 9) {
    adc->SMPR1 &= ~(0b111 << (channel * 3));       // Reset
    adc->SMPR1 |= (sample_time << (channel * 3));  // Set config
  } else {
    adc->SMPR2 &= ~(0b111 << (channel * 3));       // Reset
    adc->SMPR2 |= (sample_time << (channel * 3));  // Set config
  }
}

void adc_config_regular_channel_position(ADC_TypeDef *adc, uint8_t channel, uint8_t position) {
  // Update ADC->SQRx with the "channel" at "position" on the queue
  // NOTE(dd): DOES NOT CHECK FOR ADSTART
  uint8_t position_reg = (position % 5) * 6;  // 5 channels per register
  if (position < 5) {
    adc->SQR1 |= (channel << position_reg);
  } else if (position < 10) {
    adc->SQR2 |= (channel << position_reg);
  } else if (position < 15) {
    adc->SQR3 |= (channel << position_reg);
  } else if (position < 17) {
    adc->SQR4 |= (channel << position_reg);
  }
}

void adc_config_regular_channel(ADC_TypeDef *adc,     //
                                uint8_t channel,      //
                                uint8_t sample_time,  //
                                uint8_t position) {   //
  // Updates ADC->SMPRx with sample time and ADC->SQRx with channel position
  // NOTE(dd): DOES NOT CHECK FOR ADSTART
  adc_config_sample_time(adc, channel, sample_time);
  adc_config_regular_channel_position(adc, channel, position);
}

void adc_config_regular_sequence_length(ADC_TypeDef *adc,     //
                                        uint8_t positions) {  //
  // Update ADC->SQR1 with the number of channels to read in a sequence
  // NOTE(dd): DOES NOT CHECK FOR ADSTART
  adc->SQR1 &= ~(0b1111);      // Clear regular sequence length
  adc->SQR1 |= positions - 1;  // Update regular sequence length
}

void adc_dual_12_setup() {
  ADC1->CFGR &= ~(1U << 13);     // Single conversion mode
  ADC1->CFGR |= (0b11 << 10);    // Hardware trigger (both edges) [EXTEN]
  ADC1->CFGR |= (0b01011 << 5);  // Trigger on TIM2 TRGO [EXTSEL]
  ADC1->CFGR |= (1U);            // DMAEN=1

  // Dual mode
  ADC12_COMMON->CCR |= 0b00001;       // Enable dual mode
  ADC12_COMMON->CCR |= (1U << 13);    // DMA circular mode
  ADC12_COMMON->CCR |= (0b10 << 14);  // DMA memory both data in the same reg

  // Enable overrun interrupt (for errors)
  __disable_irq();
  // ADC1->IER |= (1U << 4);  // Enable ADC OVR interrupt
  ADC1->IER |= (1U << 2);  // Enable "End Regular Conversion" interrupt
  NVIC_EnableIRQ(ADC1_2_IRQn);
  NVIC_SetPriority(ADC1_2_IRQn, 2);
  __enable_irq();
}

void adc_dual_34_setup() {
  ADC3->CFGR &= ~(1U << 13);     // Single conversion mode
  ADC3->CFGR |= (0b11 << 10);    // Hardware trigger (both edges) [EXTEN]
  ADC3->CFGR |= (0b01011 << 5);  // Trigger on TIM2 TRGO [EXTSEL]
  ADC3->CFGR |= (1U);            // DMAEN=1

  // Dual mode
  ADC345_COMMON->CCR |= 0b00001;       // Enable dual mode
  ADC345_COMMON->CCR |= (1U << 13);    // DMA circular mode
  ADC345_COMMON->CCR |= (0b10 << 14);  // DMA memory both data in the same reg
}

/* ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
## System 2.2.1 Missed ADC triggers from TIM1/TIM8, TIM2/TIM5/TIM4/TIM6/TRGO or TGRO2 event
## Description:
The ADC external triggers for regular and injected channels by the
TIM1, TIM8, TIM2, TIM5, TIM4 and TIM6 TRGO or TRGO2 events are missed at the
following conditions:
- Prescaler enabled on the PCLK2 clock.
- TIMxCLK = 2xADCCLK and the master mode selection (MMS or MMS2 bits in the
TIMx_CR2 timer register) as reset, update, or compare pulse configuration.
- TIMxCLK = 4xADCCLK.
## Workarounds
- For TIM1 and TIM8 TRGO or TRGO 2 events: select the trigger detection on both
the rising and falling edges. The EXTEN[1:0] or JEXTEN[1:0] bits must be set to
0x11 in the ADC_CR2 register.
- For TIM2/TIM4/TIM5/TIM6/ TRGO or TGRO2 events: enable the DAC peripheral clock
in the RCC_APB1ENR register

# Start ADC up
  - exit deep-power-down mode
  - enable voltage regulator
  - wait for it to be ready (20us)

# ADC Clock
  - Can be either
    1. SYSCLK or PLL P: Better for higher F_{adc}
    2. Derived from AHB: Better for bypassing clock domain resynchronizations?

NOTE: ADCx->CCR.CKMODE can be 01 only if RCC->CFGR.HPRE=0xxx

# Calibration
  - Check it is not in deep-power-down mode
  - Check voltage regulator is on (after startup time)
  - Check ADC->CR.ADEN = 0
  - Select single ended or differential
    - ADC->CR.ADCALDIF = 0 single-ended
    - ADC->CR.ADCALDIF = 1 differential
  - Start calibration ADC->CR.ADCAL=1
  - Wait for it to stop (ADC->CR.ADCAL=0)
  - Calibration can be read from
    - ADC->CALFACT.CALFACT_S (for single-ended)
    - ADC->CALFACT.CALFACT_D (for differential)

# Enable ADC
  - ADC->CR.ADEN

# Notes on REGISTERS
  - Control bits related to conversions ONLY IF:
    - RCC->CR.ADEN=1
    - and RCC->CR.ADSTART=0
    - and RCC->CR.JADSTART=0

# Sampoling time (Ts)
  - Must be between 0.041 and 10.675 us
  - Total conversion time is Ts + 12.5 ADC clock cycles

# Starting conversions
  - Configure ADC->CFGR.EXTEN = 0b11
  - ADC->CR.ADSTART = 1 -> on trigger conversions happen

# ADC Triggers
  - For ADC1/2: adc_ext_trg3 = TIM2_CC2 (ADC->CFGR.EXTSEL=0b00011)

# Gain compensation
  - Exists :)


*/
