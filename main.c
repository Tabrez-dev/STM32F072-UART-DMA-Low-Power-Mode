#include "hal.h"
#include "uart.h"

void SystemInit(void) {
}

int main(void) {
	uartInit(UART1, 115200);

	while (1) {
		if (uartDataAvailable()) {
			uint8_t byte = uartRead();
			// Echo the received byte:
			uartStartTxDMA(byte);
		}
		//Free to do other tasks
	}
}

