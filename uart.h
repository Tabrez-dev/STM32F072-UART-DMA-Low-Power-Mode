#pragma once

#include "hal.h"

//one-byte transfers.
#define UART_TX_BUFFER_SIZE 1
#define UART_RX_BUFFER_SIZE 1
#define PACKET_SIZE         1

// Function prototypes:
void USART1_Init(USART_TypeDef *uart, unsigned long baud);
void DMA_Init(void);




