#include "stm32g4xx.h"

#define GPIO_PORT_A 0
#define GPIO_PORT_B 1
#define GPIO_PORT_C 2
#define GPIO_PORT_D 3
#define GPIO_PORT_E 4
#define GPIO_PORT_F 5
#define GPIO_PORT_G 6

enum GPIO_PORT_PIN_A{
  PA0=0,  PA1,PA2,PA3,PA4,PA5,PA6,PA7,PA8,PA9,PA10,PA11,PA12,PA13,PA14,PA15
};
enum GPIO_PORT_PIN_B{
  PB0=16, PB1,PB2,PB3,PB4,PB5,PB6,PB7,PB8,PB9,PB10,PB11,PB12,PB13,PB14,PB15
};
enum GPIO_PORT_PIN_C{
  PC0=32, PC1,PC2,PC3,PC4,PC5,PC6,PC7,PC8,PC9,PC10,PC11,PC12,PC13,PC14,PC15
};
enum GPIO_PORT_PIN_D{
  PD0=48, PD1,PD2,PD3,PD4,PD5,PD6,PD7,PD8,PD9,PD10,PD11,PD12,PD13,PD14,PD15
};
enum GPIO_PORT_PIN_E{
  PE0=64, PE1,PE2,PE3,PE4,PE5,PE6,PE7,PE8,PE9,PE10,PE11,PE12,PE13,PE14,PE15
};
enum GPIO_PORT_PIN_F{
  PF0=80, PF1,PF2,PF3,PF4,PF5,PF6,PF7,PF8,PF9,PF10,PF11,PF12,PF13,PF14,PF15
};
enum GPIO_PORT_PIN_G{
  PG0=96, PG1,PG2,PG3,PG4,PG5,PG6,PG7,PG8,PG9,PG10,PG11,PG12,PG13,PG14,PG15
};

GPIO_TypeDef * __gpio_GET_gpio_by_pin(char port_number);
void gpio_setup_pin(int PORT_PIN, char mode);
void gpio_setup_af(int PORT_PIN, char AF);
void gpio_write(int PORT_PIN, char value);
char gpio_read(int PORT_PIN);
void gpio_enable_irq(int PORT_PIN, char IS_FALLING_EDGE,
                        char IS_RISING_EDGE);
void gpio_disable_irq(int PORT_PIN, char DISABLE_HANDLER);
