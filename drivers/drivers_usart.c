#include "drivers_usart.h"

void usart_setup(USART_TypeDef* usart, uint32_t brr_value) {
  if (usart == LPUART1) {
    RCC->APB1ENR2 |= (1U);        // Enable LPUART1
    RCC->CCIPR &= ~(0b11 << 10);  // Clock source PLL P (20 MHz)
  } else if (usart == USART1) {
    RCC->APB2ENR |= (1U << 14);  // Enable USART1
    RCC->CCIPR &= ~(0b11);       // Clock source PLL P (20 MHz)
  } else if (usart == USART2) {
    RCC->APB1ENR2 |= (1U << 17);  // Enable LPUART1
    // RCC->CCIPR &= ~(0b11 << 2);  // Clock source PLL P (20 MHz)
  }
  __DSB();
  __asm__("NOP");
  __asm__("NOP");

  // Word length 8 bits (M1:M0 = 00)
  usart->CR1 &= ~(1U << 28);  // M1
  usart->CR1 &= ~(1U << 12);  // M0

  // Baud rate: (20 MHz * 256 * 2) / 88888 = 115201.152012
  // usart->PRESC |= 1U;    // Prescaler = 2
  // usart->BRR = 88888 & 0xfffff;  // BRR = 88888
  usart->BRR = brr_value;

  usart->CR2 &= ~(0b11 << 12);  // 1 stop bit

  usart->CR3 |= (1U << 12);  // Ignore overrun

  usart->CR1 |= (1U << 3);  // Enable transmission
  usart->CR1 |= (1U << 2);  // Enable reception
  usart->CR1 |= (1U);       // Enable
}

void usart_write_byte(USART_TypeDef* usart, uint8_t the_byte) {
  while (!(usart->ISR & (1U << 7))) __asm__("NOP");  // Wait for TXE
  usart->TDR = the_byte;                             // Write next bit to TDR
}

void usart_write(USART_TypeDef* usart, uint8_t char_ptr[], uint8_t size) {
  uint16_t i = 0;
  for (i = 0; i < size; i++) { usart_write_byte(usart, char_ptr[i]); }
  while (!(usart->ISR & (1U << 6))) __asm__("NOP");  // Wait for TC
}

void usart_enable_dma_tx(USART_TypeDef* usart) {
  usart->CR3 |= (1U << 7);  // RM0440.1715
}

void usart_disable_dma_tx(USART_TypeDef* usart) {
  usart->CR3 &= ~(1U << 7);  // RM0440.1715
}

void usart_enable_it_tc(USART_TypeDef* usart) {
  usart->CR1 |= (1U << 6);  // RM0440.1710
}

void usart_disable_it_tc(USART_TypeDef* usart) {
  usart->CR1 &= ~(1U << 6);  // RM0440.1710
}

void usart_enable_it_rx(USART_TypeDef* usart) {
  usart->CR1 |= (1U << 5);  // RM0440.1710
}

void usart_disable_it_rx(USART_TypeDef* usart) {
  usart->CR1 &= ~(1U << 5);  // RM0440.1710
}

/*
# To transmit a character
  - Define word length (M bits in the LPUART->CR1)
  - Select baud rate (LPUART->BRR)
  - Select stop bits (LPUART->CR2.stop)
  - Enable LPUART (LPUART->CR1.UE)
  - *** Configure DMA (LPUART->CR3.DMAT)
  - Enable tranmission (LPUART->CR1.TE)
  - Write data in LPUART->TDR
    - Wait for LPUART->ISR.TXE == 1 (LPUART->TDR empty)
    - repeat
  - Wait for LPUART->ISR.TC == 1 (tranmission complete)

# Baud Rate Calculation
  - BaudRate = (256 * LPUART->PRESC) / (LPUART->BRR)
  - 115200 = (256 * LPUART->PRESC=1) / (LPUART->BRR)

# Send data with DMA
  - Write LPUART->TDR address in the DMA control register
    (data is moved here after LPUART->ISR.TXE is set)
  - Write the address of the data on the DMA register
  - Configure total number of bytes to send (DMA)
  - Configure channel priority
  - Configure DMA interrupt after full transfer
  - Clear LPUART->ISR.TC flag (set LPUART->ICR.TCCF)
  - Activate the channel in the DMA register

# Receive data with DMA
  - Write LPUART->RDR register to DMA control register
  - Write the address of the data on the DMA register
  - Configure total number of bytes to be transfered to the DMA
  - Configure channel priority
  - Configure interrupt after full transfer
  - Activate DMA channel

*/

/*
PA3 -> LPUART1_RX [STLINKV3E_VCP_TX]
PA2 -> LPUART1_TX [STLINKV3E_VCP_RX]
Both of them are:
- Alternate function Push Pull
- No pull up and no pull down
- Max output speed: low
- Fast mode: N/A
- No NVIC and no DMA
===== LPUART 1 Configuration  ===== ===== ===== ===== ===== ===== ===== =====
-                         Mode : Asynchronous
-                        RS232 : disable
-                        RS485 : disable
===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
-                    Baud Rate : 115200 Bps
-                  Word length : 8 bits
-                       Parity : NOne
-                    Stop bits : 1
===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
-               Data direction : Receive and Transmit
-                Single sample : disable
-              Clock prescaler : 1
-                    FIFO mode : FIFO Mode Disabled
-              TxFIFO Treshold : 1 eighth full configuration
-              RxFIFO Treshold : 1 eighth full configuration
===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
- Tx pn active level inversion : disable
- Rx pn active level inversion : disable
-               Data inversion : disable
-           Tx and Rx swapping : disable
-                      Overrun : enable
-              DMA on RX error : enable
-                    MSB first : disable
===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
*/
