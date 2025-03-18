#include "hal.h"
#include "uart.h"

void SystemInit(void) {
}//startup needs this
int main(void) {
    // Initialize peripherals
    USART1_Init(UART1, 115200);
    DMA_Init();

    // Enable DMA for RX (continuous listening)
    DMA1_Channel3->CCR |= BIT(0); // Enable RX DMA
    __asm volatile (
    "ldr r0, =0xE000ED10\n\t"  // Load address of SCR into r0
    "ldr r1, [r0]\n\t"         // Read current SCR value into r1
    "movs r2, #2\n\t"          // Load constant 2 into r2
    "orr r1, r1, r2\n\t"       // Set SLEEPONEXIT bit (bit 1) in r1
    "str r1, [r0]\n\t"         // Write updated value back to SCR
        );

    while(1){
    __asm volatile("wfi");
    }
    return 0;
}

