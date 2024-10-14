# Projeto IC 2024
Códigos embarcados em um STM32 G474RET6. Códigos escritos em `C`. O código está dividido em três partes:
1. **Drivers:** Funções que manipulam os registradores do microcontrolador
2. **Libraries:** As bibliotecas escritas durante o trabalho
  - Cálculo de parâmetros de energia elétrica
  - Controles do ESP8266
  - Buffer circular
3. **Códigos:** Arquivos com os códigos `main.c` que foram compilados e executados no STM32.

# Códigos
## Monitorador Inteligente (`main_01.c`)
O principal resultado da pesquisa. O código utiliza as bibliotecas para configurar o STM32 para aquisitar dados de tensão e corrente elétrica de forma simultânea, calcular indicadores de qualidade de energia e enviar os dados para um ESP8266 (via comandos AT).
- SYSCLK em 170MHz (Utilizando PLL)
- USART1 para o ESP8266 (XXX Tx e XXX Rx)
- ADC1 no pino XXX
- ADC2 no pino XXX
- TIMER2 acionando ADC1 e ADC2 (dual mode)
- LPUART1 Comunicação com o computador via USB (debug)

## Monitorador Simples (`main_02.c`)
O código utiliza as bibliotecas para configurar o STM32 para aquisitar dados de tensão e corrente elétrica de forma simultânea e enviar os dados para o computador (script em python para receber e salvar em um arquivo).
- SYSCLK em 170MHz (Utilizando PLL)
- ADC1 no pino XXX
- ADC2 no pino XXX
- TIMER2 acionando ADC1 e ADC2 (dual mode)
- LPUART1 Comunicação com o computador via USB (debug)

# Drivers
## Clock
This module presents a function that configures the SYSCLK to 170MHz.

- PLL P =  20 MHz
- PLL Q =  85 MHz
- PLL R = 170 MHz

```c
void clock_setup();
void __clock_pll_setup();
```

## Module GPIO
### GPIO Setup
```c
void gpio_setup_pin(int PORT_PIN, char mode);
```
### GPIO Setup AF
To see correct alternate function numbers, check device datasheet.

```c
void gpio_setup_af(int PORT_PIN, char AF);
```

### GPIO Read / Write
```c
void gpio_write(int PORT_PIN, char value);
char gpio_read(int PORT_PIN);
```

### GPIO Interrupts
```c
void gpio_enable_irq(int PORT_PIN, char IS_FALLING_EDGE, char IS_RISING_EDGE);
void gpio_disable_irq(int PORT_PIN, char DISABLE_HANDLER);
```

## Module Timer 2
### Timer 2 Setup
```c
void timer_TIM2_setup();
```
### Timer 2 Start / Stop
```c
void timer_TIM2_start();
void timer_TIM2_stop();
```

## Module USART
This module sets up a "usart" instance to send data at 115200bps.

> User need to setup GPIO pins accordingly (see [GPIO Setup AF](#gpio-setup-af))

- Setup GPIO pins as Alternate Function
- Setup USART instance
- Send data as byte or as char array

```c
void usart_setup(USART_TypeDef* usart);
void usart_write_byte(USART_TypeDef* usart, uint8_t the_byte);
void usart_write(USART_TypeDef* usart, uint8_t char_ptr[], uint8_t size);
```

## Module ADC
### How to use
- Start by starting up the ADC (exit deep-power-down mode)
- Enable the ADC module
- Setup the sample time for channels
- Setup hardware trigger selection (optional)
- Start regular conversion

### ADC Startup
Exits deep-power-down mode, starts voltage regulator and does calibration process
```c
void ADC_startup(ADC_TypeDef *adc);
```
### ADC Enable / Disable
To enable: clear ADRDY flag, enable ADC and wait for startup.
To disable: check if there is nothing running (disable if needed) set ADDIS and wait for ADEN=0.
```c
void ADC_enable(ADC_TypeDef *adc);
void ADC_disable(ADC_TypeDef *adc);
```
### ADC Regular mode Start / Stop
To start: Enable ADC->CR.ADSTART.
To stop: Enable ADC->CR.ADSTP.
```c
void ADC_start_regular(ADC_TypeDef *adc);
void ADC_stop_regular(ADC_TypeDef *adc)
```
### ADC Config Sample Time
Update ADC->SMPRx with "cfg" value on "channel" position.
User must stop all regular and injected conversions before calling this.
```c
void ADC_cfg_sample_time(ADC_TypeDef *adc, uint8_t channel, uint8_t cfg);
```
### ADC  Setup 1 and 2 as dual mode
- Single conversion mode with DMA
- Trigger on rising edge of TIM2 TRGO event
- Enable dual mode (ADC 1 and 2)
- DMA circular mode with both data in the same register
- Enable interrupt `ADC1_2_IRQn` for error (overrun)
```c
void ADC_1_2_setup();
```

## Module DMA
- Set peripheral register address in DMA->CPARx
- Set CMAR
- Set DMA->CNTRx
- Configure DMA->CCRx
  - Channel priority
  - Data transfer direction
  - Circular mode
  - PERIPHERAL and MEMORY incremented mode
  - PERIPHERAL and MEMORY data size
  - Interrupt enable at half full / full transfer / transfer error
- Activate channel DMA->CCRx.EN

## Module SPI
TODO




```c
//              33222222 22221111 11111100 00000000
//              10987654 32109876 54321098 76543210
//              xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx

// Set bit 11
// REGISTRADOR: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
// (1U << 11) : 00000000 00000000 00001000 00000000
// Bitwise OR : xxxxxxxx xxxxxxxx xxxx1xxx xxxxxxxx
REGISTRADOR = REGISTRADOR | (1U << 11);
REGISTRADOR |= (1U << 11);

// Reset bit 11
// REGISTRADOR: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
// ~(1U << 11): 11111111 11111111 11110111 11111111
// Bitwise AND: xxxxxxxx xxxxxxxx xxxx0xxx xxxxxxxx
REGISTRADOR = REGISTRADOR & ~(1U << 11);
REGISTRADOR &= ~(1U << 11);
```
