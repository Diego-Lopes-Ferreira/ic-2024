#include "stm32g4xx.h"

void timer_TIM2_setup();
void timer_setup(TIM_TypeDef* tim, uint32_t pre, uint32_t arr);
void timer_start(TIM_TypeDef* tim);
void timer_stop(TIM_TypeDef* tim);
void __timer_enable_irq(TIM_TypeDef* tim);
void __timer_rcc_enable(TIM_TypeDef* tim);
