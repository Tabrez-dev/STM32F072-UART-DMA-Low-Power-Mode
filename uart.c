#include "hal.h"
#include "uart.h"
// Buffer for RX and TX
volatile uint8_t rxBuffer = 0;
volatile uint8_t txBuffer = 0;

/*
   Initialize UART and DMA for one-byte transfers.
   We configure both TX and RX DMA channels in normal (non-circular) mode.
 */
void USART1_Init(USART_TypeDef *uart, unsigned long baud) {
	(void)uart;  // For this example, we assume UART1 is used.
	uint8_t af=1;
	uint16_t rxPin=0,txPin=0;

	if (uart==UART1) {
		RCC->APB2ENR |= BIT(14);  // Enable USART1 clock
		txPin=PIN('A', 9);
		rxPin=PIN('A', 10);
	}

	// Configure TX and RX pins as alternate function
	gpio_set_mode(txPin, GPIO_MODE_AF);
	gpioSetAF(txPin, af);
	gpio_set_mode(rxPin, GPIO_MODE_AF);
	gpioSetAF(rxPin, af);

	// Configure USART1: Enable UE, TE, RE; set baud rate.
	uart->CR1 = 0;
	uart->BRR = FREQ / baud;
	uart->CR1 |= BIT(0) | BIT(2) | BIT(3);

	//Enable DMA mode in USART (DMAT for TX and DMAR for RX)
	uart->CR3 |= BIT(7) | BIT(6);
        NVIC_SetPriority(USART1_IRQn, 2);  // Lower priority (higher number)
        NVIC_EnableIRQ(USART1_IRQn);
}

void DMA_Init(void) {
    // 1. Enable the DMA1 peripheral clock
    RCC->AHBENR |= BIT(0);
    // 2. Configure RX DMA (Channel 3)
    DMA1_Channel3->CCR &= ~BIT(0);  // Disable channel before configuration
    // 3. Set peripheral address (USART1 RDR) and memory address (&rxByte)
    DMA1_Channel3->CPAR = (uint32_t)&UART1->RDR;
    DMA1_Channel3->CMAR = (uint32_t)&rxBuffer;
DMA1_Channel3->CCR &= ~((uint32_t)((0x3 << 8) | (0x3 << 10)));  // Clear PSIZE and MSIZE fields for RX DMA
DMA1_Channel3->CCR &= ~BIT(7);         // Disable MINC (memory increment)
// 4. Set number of data items to transfer = 1
    DMA1_Channel3->CNDTR = 1;
    // 5. Configure DMA Channel 3: Circular mode, transfer complete interrupt, memory increment
    DMA1_Channel3->CCR |= ~BIT(4) | BIT(1); // DIR=0(read from peripheral),TCIE



    // 6. Configure TX DMA (Channel 2)
    DMA1_Channel2->CCR &= ~BIT(0);  // Disable channel before configuration
    // 7. Set peripheral address (USART1 TDR) and memory address (&txByte)
    DMA1_Channel2->CPAR = (uint32_t)&UART1->TDR;
    DMA1_Channel2->CMAR = (uint32_t)&txBuffer;
    // 8. Set the number of data items to transfer = 1
    DMA1_Channel2->CNDTR = 1;
    
DMA1_Channel2->CCR &= ~((uint32_t)((0x3 << 8) | (0x3 << 10)));  // Clear PSIZE and MSIZE fields for TX DMA
DMA1_Channel2->CCR &= ~BIT(7);         // Disable MINC (memory increment)
// 9. Configure DMA Channel 2: Memory-to-peripheral, transfer complete interrupt
    //    Note: Memory increment (MINC) is not needed for a single byte transfer.
    DMA1_Channel2->CCR = BIT(4) | BIT(1);  // DIR and TCIE

    // 10. Enable DMA1 Channel 2/3 interrupt in NVIC
    NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1);  // Higher priority (lower number)
    NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

/*
   DMA1_Channel2_3 IRQ Handler handles both TX (Channel 2) and RX (Channel 3).
 */
// DMA1 Channel 2/3 interrupt handler
void DMA1_Channel2_3_IRQHandler(void) {
    if (DMA1->ISR & BIT(9)) { // TCIF3 (RX complete)
        DMA1->IFCR = BIT(9); // Clear TCIF3
        txBuffer = rxBuffer; // Echo the character

        // Start TX DMA
        DMA1_Channel2->CNDTR=1; // 1 byte to transmit
        DMA1_Channel2->CCR |= BIT(0);// Enable TX DMA
    }
    if (DMA1->ISR & BIT(5)) { // TCIF2 (TX complete)
        DMA1->IFCR = BIT(5); // Clear TCIF2
        DMA1_Channel2->CCR &= ~BIT(0); // Disable TX DMA
        
    }
}

// USART1 interrupt handler (for errors)
void USART1_IRQHandler(void) {
    if (USART1->ISR & BIT(3)) { // ORE (overrun error)
        USART1->ICR |= BIT(3); // Clear ORE flag
    }
}

