#include "stm32g474xx.h"

void usart_setup(USART_TypeDef* usart, uint32_t brr_value);

void usart_enable_dma_tx(USART_TypeDef* usart);
void usart_disable_dma_tx(USART_TypeDef* usart);

void usart_enable_it_tc(USART_TypeDef* usart);
void usart_disable_it_tc(USART_TypeDef* usart);

void usart_enable_it_rx(USART_TypeDef* usart);
void usart_disable_it_rx(USART_TypeDef* usart);

void usart_write_byte(USART_TypeDef* usart, uint8_t the_byte);
void usart_write(USART_TypeDef* usart, uint8_t char_pointer[], uint8_t size);
