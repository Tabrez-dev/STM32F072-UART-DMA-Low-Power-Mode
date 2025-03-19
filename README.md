### **Efficient and Low-Power UART Communication with DMA on STM32F072**  

### **Overview**  
This project implements a **fully interrupt-driven UART DMA communication** on the STM32F072RBT6 Discovery board, ensuring minimal CPU intervention and optimized power efficiency. Unlike traditional polling or blocking UART methods, this approach leverages **Direct Memory Access (DMA)** to handle both transmission (TX) and reception (RX), reducing CPU load and allowing the microcontroller to operate in a **low-power mode (Sleep-on-Exit).**  

The implementation follows **bare-metal programming principles** using the **CMSIS** library for direct hardware control without relying on external HAL libraries.  

---

## **Technical Highlights**  

### **1. Fully Interrupt-Driven UART with DMA**  
- **TX and RX operations are completely offloaded to DMA** to eliminate CPU intervention.  
- **USART1 peripheral is configured with DMA mode enabled** for seamless data transfer.  
- The **DMA interrupt (TCIE - Transfer Complete Interrupt) is used** to signal when a byte has been sent or received.  
- **Circular mode is enabled for RX DMA**, allowing continuous reception without reinitialization.  

### **2. Optimized Low-Power Design**  
- The microcontroller **enters Sleep mode (WFI - Wait For Interrupt) when idle**, significantly reducing power consumption.  
- The **Sleep-on-Exit (SCR register modification) ensures the CPU wakes only for relevant interrupts** and automatically returns to sleep afterward.  
- DMA operation allows **peripherals to continue running independently of the CPU**, ensuring efficient data transfer while the system remains in low-power mode.  

### **3. Robust UART Error Handling**  
- The **USART1 interrupt handles error conditions (Overrun, Framing, Noise, and Parity errors)** to maintain reliable communication.  
- Error flags are cleared in the ISR to **prevent UART stalls or data corruption**.  

### **4. DMA Prioritization for Reliable Data Handling**  
- **NVIC priorities are carefully assigned** to ensure **DMA interrupts (high priority) are processed before USART errors (lower priority).**  
- This ensures **data integrity and uninterrupted communication**, even under high-speed operation.  

### **5. Minimal Resource Usage & CMSIS-Based Implementation**  
- **No external libraries (except nano libc for `printf`) are used**, ensuring a **lightweight, efficient** implementation.  
- **Register-level configuration using CMSIS** ensures **maximum control, portability, and performance**.  

---

## **Why This Approach?**  
| Feature | Traditional UART (Polling/Blocking) | This Project (UART with DMA) |  
|---------|----------------------------------|------------------------------|  
| **CPU Usage** | High (Polling blocks execution) | Minimal (DMA handles transfers) |  
| **Power Efficiency** | Low (CPU remains active) | High (MCU sleeps between events) |  
| **Interrupt Usage** | Only for error handling | Fully interrupt-driven |  
| **Data Throughput** | Lower (CPU bottleneck) | Higher (DMA offloads CPU) |  
| **Scalability** | Limited to single-task systems | Easily extendable for multi-tasking |  

---

## **How It Works**  
### **1. Initialization**  
- USART1 is configured for **115200 baud, 8N1 format, and DMA-enabled TX/RX**.  
- DMA1 Channel 3 (RX) and DMA1 Channel 2 (TX) are set up for **single-byte transfers**.  

### **2. RX DMA Operation**  
- The **USART1 RX DMA continuously listens for incoming data** and stores it in a buffer.  
- **DMA automatically reloads (circular mode)** after each received byte, ensuring uninterrupted reception.  

### **3. TX DMA Operation**  
- On receiving a byte, the **DMA interrupt triggers an echo back** by copying the received byte into the TX buffer.  
- The **TX DMA is enabled only when data needs to be sent**, ensuring power efficiency.  

### **4. Sleep Mode Integration**  
- The **MCU enters low-power mode (`WFI` instruction) whenever idle**.  
- It **wakes up only when DMA or USART interrupts occur**, minimizing power consumption.

## Leveraging Sleep Mode in STM32F072RBT6 for Enhanced Power Efficiency

In my implementation, I have strategically utilized the STM32F072RBT6's Sleep mode to optimize power consumption during idle periods. Below is an overview of my approach:

### 1. Enabling Sleep-on-Exit Feature

I have configured the microcontroller to automatically enter Sleep mode upon exiting an interrupt service routine (ISR). This is achieved by setting the **SLEEPONEXIT** bit in the System Control Register (SCR). Activating this feature ensures that after an interrupt is serviced, the CPU returns to Sleep mode without executing additional instructions, thereby minimizing unnecessary CPU activity and reducing power consumption. 

### 2. Implementing Wait-for-Interrupt in Main Loop

In the main function, I have implemented an infinite loop that continuously executes the `WFI` (Wait For Interrupt) instruction. The `WFI` instruction places the CPU into Sleep mode until an interrupt occurs. This design ensures that the microcontroller remains in a low-power state during idle periods, waking up only to handle interrupts. Such a strategy is particularly beneficial in applications where the CPU is not required to run continuously, leading to significant energy savings.

---

## **Future Improvements**  
- **Buffering for multi-byte messages** instead of single-byte transactions.  
---

This README presents your project in a professional and structured manner, highlighting the technical decisions that demonstrate **industry best practices, efficiency, and robustness**. 🚀
