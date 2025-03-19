#include "hal.h"
#include "uart.h"


void SystemInit(void) {
    // System-level initialization typically done here (e.g., clock configuration)
}


int main(void) {
    // Initialize USART1 with a baud rate of 115200 for UART communication
    USART1_Init(UART1, 115200);
    // Initialize DMA for RX and TX channels to handle UART communication efficiently
    DMA_Init();

    // Enable DMA for RX to start listening for incoming data
    DMA1_Channel3->CCR |= BIT(0); // Enable RX DMA
    
    // Inline assembly to configure the Sleep-on-Exit feature:
    // This instructs the MCU to enter low-power mode when returning from an interrupt.
    __asm volatile (
    "ldr r0, =0xE000ED10\n\t"  // Load address of SCR (System Control Register)
    "ldr r1, [r0]\n\t"         // Read current SCR value into r1
    "movs r2, #2\n\t"          // Load constant 2 into r2 (Sleep-on-Exit bit)
    "orr r1, r1, r2\n\t"       // Set SLEEPONEXIT bit (bit 1) in r1
    "str r1, [r0]\n\t"         // Write updated value back to SCR
        );
    // Infinite loop that waits for interrupts, putting the system in low-power mode when idle
    while(1){
    __asm volatile("wfi");// Wait For Interrupt (low-power mode until an interrupt occurs)
                          // The CPU will resume execution from here once the interrupt is served
                          // CPU halts but peripherals are still active.
    }
    return 0;// The main function will never return as the system runs indefinitely
}

