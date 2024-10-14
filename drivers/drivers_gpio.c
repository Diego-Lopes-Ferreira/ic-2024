#include "drivers_gpio.h"

#include "stm32g4xx.h"


// NOTE(dd): PB8 and PG10 are boot gpios DONT USE THEM

GPIO_TypeDef* __gpio_GET_gpio_by_pin(char port_number) {
  GPIO_TypeDef* port;

  switch (port_number) {
    case GPIO_PORT_A:
      port = GPIOA;
      break;
    case GPIO_PORT_B:
      port = GPIOB;
      break;
    case GPIO_PORT_C:
      port = GPIOC;
      break;
    case GPIO_PORT_D:
      port = GPIOD;
      break;
    case GPIO_PORT_E:
      port = GPIOE;
      break;
    case GPIO_PORT_F:
      port = GPIOF;
      break;
    case GPIO_PORT_G:
      port = GPIOG;
      break;
  }
  return port;
}

void gpio_setup_pin(int PORT_PIN, char mode) {
  /* Modes
    00: Input mode (digital)
    01: Output mode (digital)
    10: Alternate function
    11: Analog mode (reset state) */

  // Get information and pointer to GPIO
  char port = PORT_PIN / 16;
  char pin = PORT_PIN % 16;
  GPIO_TypeDef* GPIO = __gpio_GET_gpio_by_pin(port);

  // Enable GPIO clock in RCC
  RCC->AHB2ENR |= (1 << port);
  __DSB();

  // Change mode
  GPIO->MODER &= ~(0b11 << (pin * 2));  // Reset
  GPIO->MODER |= (mode << (pin * 2));   // Set mode

  // Set as push pull (not open drain)
  GPIO->OTYPER &= ~(0b1 << pin);  // output as push-pull

  // Change speed (to f.a.s.t. tm)
  GPIO->OSPEEDR &= ~(0b11 << (pin * 2));  // Reset
  GPIO->OSPEEDR |= (0b11 << pin * 2);     // Set speed as f.a.s.t.

  // Configure pull-up or pull-down
  GPIO->PUPDR &= ~(0b11 << (pin * 2));  // Reset (no pull up and no pull down)
  // GPIO->PUPDR |= (0b10 << pin * 2);     // Set as pull down
}

void gpio_setup_af(int PORT_PIN, char AF) {
  gpio_setup_pin(PORT_PIN, 0b10);  // this is kinda bad, but oke ;)

  // Get information and pointer to GPIO
  char port = PORT_PIN / 16;
  char pin = PORT_PIN % 16;
  GPIO_TypeDef* GPIO = __gpio_GET_gpio_by_pin(port);

  // Setup alternate function
  if (pin > 7) {
    GPIO->AFR[1] &= ~(0b1111 << (pin * 4));  // Reset
    GPIO->AFR[1] |= (AF << (pin * 4));       // Set AF
  } else {
    GPIO->AFR[0] &= ~(0b1111 << (pin * 4));  // Reset
    GPIO->AFR[0] |= (AF << (pin * 4));       // Set AF
  }
}

void gpio_write(int PORT_PIN, char value) {
  // Get information and pointer to GPIO
  char port = PORT_PIN / 16;
  char pin = PORT_PIN % 16;
  GPIO_TypeDef* GPIO = __gpio_GET_gpio_by_pin(port);

  // Request change in value from "bit set/reset register"
  if (value == 1) {
    GPIO->BSRR |= (1U << (pin));  // Set HIGH
  } else {
    GPIO->BSRR |= (1U << (pin + 16));  // Set LOW
  }
}

char gpio_read(int PORT_PIN) {
  // Get information and pointer to GPIO
  char port = PORT_PIN / 16;
  char pin = PORT_PIN % 16;
  GPIO_TypeDef* GPIO = __gpio_GET_gpio_by_pin(port);

  if ((GPIO->IDR & (1U << pin)) > 0) {
    return 1;
  } else {
    return 0;
  }
}

void gpio_enable_irq(int PORT_PIN, char IS_FALLING_EDGE,
                        char IS_RISING_EDGE) {
  /* STM32 G4 74RE Reference Manual (445)
     - EXTI lines 0 to 15 are for GPIO
     - Pin must be set as input
     - Must have IRQ function implemented
   */

  // Get information and pointer to GPIO
  char port = PORT_PIN / 16;
  char pin = PORT_PIN % 16;
  // GPIO_TypeDef * GPIO = __gpio_GET_gpio_by_pin(port);

  // gpio_setup_pin(PORT_PIN, 0b00);  // Set pin as input?

  // Enable SYSFG clock
  RCC->APB2ENR |= 1U;  // Enable SYSCFG

  // Configure trigger selection (RTSR and FTSR)
  if (IS_FALLING_EDGE) {
    EXTI->FTSR1 &= ~(1U << pin);  // Reset
    EXTI->FTSR1 |= (1U << pin);   // Set
  }
  if (IS_RISING_EDGE) {
    EXTI->RTSR1 &= ~(1U << pin);  // Reset
    EXTI->RTSR1 |= (1U << pin);   // Set
  }

  // Enable NVIC channel
  unsigned char exti_reg = pin / 4;  // Each EXTI register has 4 pins
  unsigned char exti_pin = pin % 4;  // Pin location is one of [0,1,2,3]
  SYSCFG->EXTICR[exti_reg] &= (0b1111 << (exti_pin * 4));  // Reset
  SYSCFG->EXTICR[exti_reg] |=
      (port << (exti_pin * 4));  // Set PORT on PIN position

  // Enable handler function
  __disable_irq();
  if (pin == 0) {
    NVIC_EnableIRQ(EXTI0_IRQn);
  } else if (pin == 1) {
    NVIC_EnableIRQ(EXTI1_IRQn);
  } else if (pin == 2) {
    NVIC_EnableIRQ(EXTI2_IRQn);
  } else if (pin == 3) {
    NVIC_EnableIRQ(EXTI3_IRQn);
  } else if (pin == 4) {
    NVIC_EnableIRQ(EXTI4_IRQn);
  } else if (pin < 10) {
    NVIC_EnableIRQ(EXTI9_5_IRQn);
  } else {
    NVIC_SetPriority(EXTI15_10_IRQn, 0);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
  }
  __enable_irq();

  // Enable interrupt (Configure mask bit (IMR))
  EXTI->IMR1 |= (1U << pin);  // Set
}

void gpio_disable_irq(int PORT_PIN, char DISABLE_HANDLER) {
  /* STM32 G4 74RE Reference Manual (445)
     - DISABLE_HANDLER: if 1 also disable with NVIC_DisableIRQ
     .                  should be 1 most of the time
     .                  put 0 when using more than one interrupt from 5 to 15
    */

  // Get information and pointer to GPIO
  // char port = PORT_PIN / 16;
  char pin = PORT_PIN % 16;
  // GPIO_TypeDef * GPIO = __gpio_GET_gpio_by_pin(port);

  // Disable interrupt (Configure mask bit (IMR))
  EXTI->IMR1 &= ~(1U << pin);  // Reset

  // Configure trigger selection (RTSR and FTSR)
  EXTI->FTSR1 &= ~(1U << pin);  // Reset falling edge
  EXTI->RTSR1 &= ~(1U << pin);  // Reset rising edge

  // Disable NVIC channel
  unsigned char exti_reg = pin / 4;  // Each EXTI register has 4 pins
  unsigned char exti_pin = pin % 4;  // Pin location is one of [0,1,2,3]
  SYSCFG->EXTICR[exti_reg] &=
      (0b1111 << (exti_pin * 4));  // Reset PORT on PIN position

  if (DISABLE_HANDLER == 0) return;
  // Disable handler function
  __disable_irq();
  if (pin == 0) {
    NVIC_DisableIRQ(EXTI0_IRQn);
  } else if (pin == 1) {
    NVIC_DisableIRQ(EXTI1_IRQn);
  } else if (pin == 2) {
    NVIC_DisableIRQ(EXTI2_IRQn);
  } else if (pin == 3) {
    NVIC_DisableIRQ(EXTI3_IRQn);
  } else if (pin == 4) {
    NVIC_DisableIRQ(EXTI4_IRQn);
  } else if (pin < 10) {
    NVIC_DisableIRQ(EXTI9_5_IRQn);
  } else {
    NVIC_DisableIRQ(EXTI15_10_IRQn);
  }
  __enable_irq();
}
