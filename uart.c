#include "hal.h"
#include "uart.h"

// One-byte variables for TX and RX
volatile uint8_t txByte;  // TX data storage
volatile uint8_t rxByte;  // RX data storage

// Flags to signal completion of DMA transfers
volatile bool txDone=false;
volatile bool rxDone=false;

/*
   Initialize UART and DMA for one-byte transfers.
   We configure both TX and RX DMA channels in normal (non-circular) mode.
 */
void uartInit(USART_TypeDef *uart, unsigned long baud) {
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

	//1. Enable DMA mode in USART (DMAT for TX and DMAR for RX)
	uart->CR3 |= BIT(7) | BIT(6);

	//2. Enable DMA clock (assuming DMA1 is enabled at RCC->AHBENR BIT(0))
	RCC->AHBENR |= BIT(0);

	//3. Enable NVIC for DMA1 Channel2_3 interrupt
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

	//4. --- Configure RX DMA (Channel 3) in Normal Mode ---
	// Disable Channel 3 before configuration
	DMA1_Channel3->CCR &= ~BIT(0);
	//5. Set peripheral address (UART1 RDR) and memory address (&rxByte)
	DMA1_Channel3->CPAR = (uint32_t)&UART1->RDR;
	DMA1_Channel3->CMAR = (uint32_t)&rxByte;
	//6. Set number of data items to transfer = 1
	DMA1_Channel3->CNDTR = 1;
	//7. Configure Channel 3: enable Transfer Complete Interrupt (TCIE).
	// Do NOT enable CIRC mode.
	DMA1_Channel3->CCR = BIT(1);  // TCIE only; MINC is optional for 1 byte.
				      //8. Enable DMA Channel 3
	DMA1_Channel3->CCR |= BIT(0);
}

/*
   Start a TX DMA transfer to send one byte.
   This function loads the data into txByte and configures DMA Channel 2.
 */
void uartStartTxDMA(uint8_t data) {
	// Wait until any previous TX transfer is complete
	while (!txDone && (DMA1_Channel2->CCR & BIT(0)));
	txDone = false;  // Clear flag

	// Load data into txByte
	txByte = data;

	//1. Disable DMA Channel 2 before configuration
	DMA1_Channel2->CCR &= ~BIT(0);
	//2. Set peripheral address (UART1 TDR) and memory address (&txByte)
	DMA1_Channel2->CPAR=(uint32_t)&UART1->TDR;
	DMA1_Channel2->CMAR=(uint32_t)&txByte;
	//3. Set the number of data items to transfer = 1
	DMA1_Channel2->CNDTR=1;
	//4. Configure Channel 2: set direction (memory-to-peripheral) and enable TCIE.
	//5. MINC is not needed for a single byte.
	DMA1_Channel2->CCR = BIT(4) | BIT(1);  // DIR and TCIE
	//6. Enable DMA Channel 2
	DMA1_Channel2->CCR |= BIT(0);
}

/*
   Returns true if a new RX byte has been received.
 */
bool uartDataAvailable(void) {
	return rxDone;
}

/*
   Reads the received byte and clears the rxDone flag.
 */
uint8_t uartRead(void) {
	rxDone=false;
	return rxByte;
}

/*
   DMA1_Channel2_3 IRQ Handler handles both TX (Channel 2) and RX (Channel 3).
 */
void DMA1_Channel2_3_IRQHandler(void) {
	// --- Handle TX DMA (Channel 2) ---
	// TCIF2 flag for Channel 2 is BIT 5
	if (DMA1->ISR & BIT(5)) {
		DMA1->IFCR |= BIT(5);  // Clear TX transfer complete flag
		txDone = true;         // Mark TX as done
	}

	// --- Handle RX DMA (Channel 3) ---
	// TCIF3 flag for Channel 3 is BIT(9)
	if (DMA1->ISR & BIT(9)) {
		DMA1->IFCR |= BIT(9);  // Clear RX transfer complete flag
		rxDone = true;         // A new byte has been received

		// Reconfigure RX DMA for the next one-byte transfer.
		DMA1_Channel3->CCR &= ~BIT(0);  // Disable Channel 3
		DMA1_Channel3->CNDTR = 1;         // Reset transfer count to 1
		DMA1_Channel3->CCR |= BIT(0);     // Re-enable Channel 3
	}
}

