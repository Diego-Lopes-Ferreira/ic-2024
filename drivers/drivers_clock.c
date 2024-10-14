#include "drivers_clock.h"

/*
HSE = 24 MHz

# From HSI16 -> PLL (higher than 80MHz)
  - Set AHB prescaler HPRE[3:0] to 2
  - Switch to PLL
  - Wait for 1us
  - Reconfigure AHB prescaler

# From <24MHz to >24MHz
  - Program the new number of wait states
  - Check the number of wait states (read FLASH->ACR)
  - Change the CPU clock source
  - Check AHB prescaler and SWS bits (read RCC->CFGR)

# Range 1 (boost mode): needs 4 wait cycles in FLASH->ACR
*/

void clock_setup() {
  uint32_t i = 0;

  RCC->CR |= (1U << 16);                 // Enable HSE
  while ((RCC->CR & (1U << 17)) == 0) {  // Wait for HSERDY
  }

  FLASH->ACR = (FLASH->ACR & ~(0x1)) | (0b0100);  // 4 wait states
  while ((FLASH->ACR & 0xf) != 0b0100) {          // Wait for value to hold
  }

  __clock_pll_setup();  // Configure PLL

  RCC->CFGR &= ~(0b1111 << 4);  // Reset AHB prescaler
  RCC->CFGR |= (0b1000 << 4);   // Change AHB prescaler to 2
  RCC->CFGR |= 11;              // Select PLL as SYSCLK
  while (i < 200) {             // Wait 1us (actually 170 should be fine)
    i++;
  }
  RCC->CFGR &= ~(0b1111 << 4);  // Change AHB prescaler to 0
  // RCC->CFGR &= ~(0b111 << 8);   // Reset APB1 prescaler
  // RCC->CFGR |= (0b000 << 8);    // Change APB1 prescaler to ?
  // RCC->CFGR &= ~(0b111 << 11);  // Reset APB2 prescaler
  // RCC->CFGR |= (0b000 << 11);   // Change APB2 prescaler to ?
}

void __clock_pll_setup() {
  // VCOi = HSE / PLLM
  // VCOo = VCOi * PLLN (with 8 <= PLLN <= 127)
  // => PLLo = [ (HSE / PLLM) * PLLN ] / prescaller of output
  // => => PLLP = [ (24 / 6) * 85 ] / 17 =  20 MHz
  // => => PLLQ = [ (24 / 6) * 85 ] / 4  =  85 MHz
  // => => PLLR = [ (24 / 6) * 85 ] / 2  = 170 MHz

  RCC->PLLCFGR &= ~(1U << 16);  // PLL P disable (adc)
  RCC->PLLCFGR &= ~(1U << 20);  // PLL Q disable (USB)
  RCC->PLLCFGR &= ~(1U << 24);  // PLL R disable (SYSCLK)

  RCC->CR &= ~(1U << 24);         // Disable PLL
  while (RCC->CR & (1U << 25)) {  // Wait for PLL to turn off
  }

  RCC->PLLCFGR |= 0b11;              // PLL source HSE
  RCC->PLLCFGR |= (0b0101 << 4);     // PLL prescaler (PLLM=6)
  RCC->PLLCFGR |= (0b1010101 << 8);  // PLL multiplication factor (PLLN=85)
  RCC->PLLCFGR |= (0b1 << 17);       // PLL P division factor (adc) = 17
  RCC->PLLCFGR |= (0b01 << 21);      // PLL Q division factor (USB) = 4
  RCC->PLLCFGR &= ~(0b11 << 25);     // PLL R division factor (SYSCLK) = 2

  RCC->CR |= (1U << 24);                 // Enable PLL
  while ((RCC->CR & (1U << 25)) == 0) {  // Wait for PLL to turn on
  }

  RCC->PLLCFGR |= (1U << 16);  // PLL P enable (adc)
  RCC->PLLCFGR |= (1U << 20);  // PLL Q enable (USB)
  RCC->PLLCFGR |= (1U << 24);  // PLL R enable (SYSCLK)
}
