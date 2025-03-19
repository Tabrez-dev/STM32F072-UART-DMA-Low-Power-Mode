#include "hal.h"
#include "uart.h"

// Buffers for receiving (RX) and transmitting (TX) single bytes.
// Marked volatile because they’re modified by DMA/interrupts and accessed elsewhere.
volatile uint8_t rxBuffer = 0;
volatile uint8_t txBuffer = 0;
/*
   Initializes USART1 for UART communication with DMA support.
   Configures TX/RX pins, baud rate, and enables DMA mode for efficient single-byte transfers.
*/
void USART1_Init(USART_TypeDef *uart, unsigned long baud) {
    (void)uart;  // Unused parameter; kept for potential future expansion to other UARTs.
    uint8_t af=1;// Alternate function 1 (AF1) maps PA9/PA10 to USART1 per datasheet.
    uint16_t rxPin=0,txPin=0;
// Hardcoded to USART1 for this project; could be expanded for other UARTs.
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

    // Configure USART1 settings for 8N1 (8 data bits, no parity, 1 stop bit).
    uart->CR1 = 0;      // Reset control register to clear prior settings.                      // Set baud rate (8 MHz / 115200 ≈ 417).
    uart->BRR = FREQ / baud;
    uart->CR1 |= BIT(0) | BIT(2) | BIT(3);//Enable USART (UE), Receiver (RE), Transmitter (TE).

    //Enable DMA mode in USART (DMAT for TX and DMAR for RX)
    uart->CR3 |= BIT(7) | BIT(6);
// Set up USART1 interrupt for error handling (e.g., overrun).
    NVIC_SetPriority(USART1_IRQn, 2);  // Lower priority (2) vs. DMA (1) to prioritize data over errors.
    NVIC_EnableIRQ(USART1_IRQn);// Enable interrupt in NVIC.
}

/*
   Initializes DMA1 Channels 2 (TX) and 3 (RX) for single-byte UART transfers.
   RX uses circular mode i.e auto-restart; TX is triggered per echo.
   
*/
void DMA_Init(void) {
    // 1. Enable the DMA1 peripheral clock
    RCC->AHBENR |= BIT(0);
    // 2. Configure RX DMA (Channel 3) to move data from USART1 to memory
    DMA1_Channel3->CCR &= ~BIT(0);  // Disable channel before configuration
    // 3. Set peripheral address (USART1 RDR) and memory address (&rxByte)
    DMA1_Channel3->CPAR = (uint32_t)&UART1->RDR;//Source: USART1 receive data register.
    DMA1_Channel3->CMAR = (uint32_t)&rxBuffer;// Destination: rxBuffer in memory.
    DMA1_Channel3->CCR &= ~((uint32_t)((0x3 << 8) | (0x3 << 10)));// Clear PSIZE/MSIZE (default to 8-bit).
    DMA1_Channel3->CCR &= ~BIT(7);// Disable memory increment (single-byte target).
    // 4. Set number of data items to transfer = 1
    DMA1_Channel3->CNDTR = 1;// Transfer 1 byte per trigger.
    // 5. Configure DMA Channel 3: Circular mode, transfer complete interrupt, memory increment
    DMA1_Channel3->CCR &= ~BIT(4);  // DIR = 0 (peripheral to memory)
    DMA1_Channel3->CCR |= BIT(5) | BIT(1); // DIR=0(read from peripheral),Enable transfer complete interrupt (TCIE).



    // 6. Configure TX DMA (Channel 2)
    DMA1_Channel2->CCR &= ~BIT(0);  // Disable channel before configuration
    // 7. Destination: USART1 transmit data register and Source: txBuffer in memory.
    DMA1_Channel2->CPAR = (uint32_t)&UART1->TDR;
    DMA1_Channel2->CMAR = (uint32_t)&txBuffer;
    // 8. Set the number of data items to transfer = 1
    DMA1_Channel2->CNDTR = 1;

    DMA1_Channel2->CCR &= ~((uint32_t)((0x3 << 8) | (0x3 << 10)));  // Clear PSIZE and MSIZE fields for TX DMA
    DMA1_Channel2->CCR &= ~BIT(7);// Disable memory increment (single-byte source).
    // 9. Configure DMA Channel 2: Memory-to-peripheral, transfer complete interrupt
    //    Note: Memory increment (MINC) is not needed for a single byte transfer.
    DMA1_Channel2->CCR = BIT(4) | BIT(1);  // Direction: memory-to-peripheral (DIR = 1), TCIE.

    // 10. Enable DMA1 Channel 2/3 interrupt in NVIC
    NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1);// Higher priority (1) for data handling vs. USART (2).
    NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);// Enable combined Channel 2/3 interrupt.
}

/*
   DMA1_Channel2_3 IRQ Handler handles both TX (Channel 2) and RX (Channel 3).
   */
void DMA1_Channel2_3_IRQHandler(void) {
// RX complete: character received from USART1.
    if (DMA1->ISR & BIT(9)) { // Check TCIF3 (Transfer Complete Interrupt Flag for Channel 3).
        DMA1->IFCR = BIT(9); // Clear TCIF3 to acknowledge interrupt.
        txBuffer = rxBuffer; // Copy received byte to TX buffer for echoing.

        // Trigger TX DMA to send the echoed character.
        DMA1_Channel2->CNDTR=1; // 1 byte to transmit
        DMA1_Channel2->CCR |= BIT(0);// Enable TX DMA
    }
    // TX complete: character has been sent.
    if (DMA1->ISR & BIT(5)) { // Check TCIF2 (Transfer Complete Interrupt Flag for Channel 2).
        DMA1->IFCR = BIT(5); // Clear TCIF2 to acknowledge interrupt.
        DMA1_Channel2->CCR &= ~BIT(0); // Disable TX DMA channel until next echo.

    }
}

/*
   USART1 interrupt handler for error conditions.
*/

void USART1_IRQHandler(void) {
    if (USART1->ISR & BIT(3)) {  // ORE (Overrun Error)
        USART1->ICR |= BIT(3);  // Clear ORE flag
    }
    if (USART1->ISR & BIT(4)) {  // FE (Framing Error)
        USART1->ICR |= BIT(4);  // Clear FE flag
    }
    if (USART1->ISR & BIT(5)) {  // NE (Noise Error)
        USART1->ICR |= BIT(5);  // Clear NE flag
    }
    if (USART1->ISR & BIT(6)) {  // PE (Parity Error)
        USART1->ICR |= BIT(6);  // Clear PE flag
    }
}

