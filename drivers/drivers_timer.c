#include "drivers_timer.h"

void timer_TIM2_setup() { timer_setup(TIM2, 170, 1000000); }

void timer_setup(TIM_TypeDef* tim, uint32_t pre, uint32_t arr) {
  // Supported TIM2, TIM3 and TIM4
  __timer_rcc_enable(tim);

  tim->CNT = 0;
  tim->PSC = (uint32_t)(pre - 1);  // Prescaler 170
  tim->ARR = (uint32_t)(arr);      // Count limit

  tim->CR2 &= ~(0b1111 << 4);  // Reset
  tim->CR2 |= (0b0010 << 4);   // Update event as TRGO

  // __timer_enable_irq(tim);

  tim->EGR |= 1U;  // Update reset counter
}

void timer_start(TIM_TypeDef* tim) { tim->CR1 |= 1U; }

void timer_stop(TIM_TypeDef* tim) {
  tim->CR1 &= ~(1U);
  tim->SR &= ~(1U);  // Clear flag
}

void __timer_enable_irq(TIM_TypeDef* tim) {
  tim->DIER |= 1U;  // Enable interrupts
  __disable_irq();
  if (tim == TIM2) {
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 2);
  } else if (tim == TIM3) {
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_SetPriority(TIM3_IRQn, 2);
  } else if (tim == TIM4) {
    NVIC_EnableIRQ(TIM4_IRQn);
    NVIC_SetPriority(TIM4_IRQn, 2);
  }
  tim->SR &= ~(1U);  // Clear flag
  __enable_irq();
}

void __timer_rcc_enable(TIM_TypeDef* tim) {
  if (tim == TIM2) {
    RCC->APB1ENR1 |= 1U;
  } else if (tim == TIM3) {
    RCC->APB1ENR1 |= (1U << 1);
  } else if (tim == TIM4) {
    RCC->APB1ENR1 |= (1U << 2);
  }
}

/*
0 -> TIM2
1 -> TIM3
2 -> TIM4
3 -> TIM5
4 -> TIM6
5 -> TIM7
*/
