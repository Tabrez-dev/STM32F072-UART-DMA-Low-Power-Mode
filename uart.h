#pragma once

#include "hal.h"

//one-byte transfers.
#define UART_TX_BUFFER_SIZE 1
#define UART_RX_BUFFER_SIZE 1
#define PACKET_SIZE         1

// One-byte variables for TX and RX
extern volatile uint8_t txByte;  // Stores the byte to be transmitted via DMA
extern volatile uint8_t rxByte;  // Stores the received byte via DMA

// Flags indicating that a DMA transfer has completed
extern volatile bool txDone;     // Set true when TX DMA transfer is complete
extern volatile bool rxDone;     // Set true when RX DMA transfer is complete

// Function prototypes:
void uartInit(USART_TypeDef *uart, unsigned long baud);
void uartStartTxDMA(uint8_t data);

bool uartDataAvailable(void); // Returns true if a new RX byte is available
uint8_t uartRead(void);         // Reads the received byte and clears the flag


