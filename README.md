# **STM32 UART with DMA (Bare-Metal Implementation)**

## **Overview**
This project demonstrates a **bare-metal implementation** of **UART communication using DMA (Direct Memory Access)** on an **STM32F072** microcontroller. The goal is to achieve efficient, **non-blocking** transmission and reception of data over UART without CPU intervention during data transfer.

---

## Other UART-based Projects

Here are some related projects I have worked on, which implement UART communication using different approaches and techniques:

1. **[STM32 UART Interrupt-Based Driver](https://github.com/Tabrez-dev/STM32-UART-Interrupt-Based-Driver)**: This project demonstrates how to implement UART communication using interrupts on STM32 microcontrollers.

2. **[STM32 Printf UART Redirection Using CMSIS (Cortex-M0)](https://github.com/Tabrez-dev/STM32-Printf-UART-Redirection-Using-CMSIS-Cortex-M0)**: A project that shows how to use CMSIS for redirection of `printf` output to UART for debugging on STM32 Cortex-M0 microcontrollers.

3. **[Baremetal Printf to UART](https://github.com/Tabrez-dev/Baremetal-Printf-to-UART)**: This project explains how to implement `printf` redirection to UART in a bare-metal environment without using any libraries.

4. **[STM32 Baremetal UART Device Driver from Scratch](https://github.com/Tabrez-dev/STM32-Baremetal-UART-Device-Driver-from-Scratch)**: A bare-metal UART driver implementation for STM32 from scratch, focusing on low-level hardware access and driver development.

5. **[STM32 UART Flashing](https://github.com/Tabrez-dev/STM32-UART-Flashing)**: This project demonstrates how to flash firmware to an STM32 microcontroller via UART using the built-in bootloader and `stm32flash`.  

## **Flowchart of code execution**

This program echoes bytes from keyboard to board and back to a serial monitor

What happens when you send a charcater from the keyboard?

[Start]
   ↓
[UART Initialized with DMA]
   ↓
[DMA RX Channel 3 Configured]
   ├── CPAR = UART1->RDR  (Peripheral: UART RX Register)
   ├── CMAR = &rxByte  (Memory: Variable to store data)
   ├── CNDTR = 1  (Transfer 1 byte)
   ├── Enable Transfer Complete Interrupt (TCIE)
   ├── Enable DMA RX Channel
   ↓
[Waiting for Data from PC]
   ↓ (PC Sends a Character)
[UART Receives Character in RDR Register]
   ↓ (DMA Automatically Moves Byte)
[DMA Transfers Byte to rxByte in Memory]
   ↓
[DMA Triggers Transfer Complete Interrupt]
   ↓
[Jump to DMA1_Channel2_3_IRQHandler()]
   ├── Clear RX Transfer Complete Flag
   ├── Set rxDone = true  (Indicates Data is Available)
   ├── Restart DMA RX Channel for Next Byte
   ├── CNDTR Reset to 1
   ├── Enable DMA RX Channel
   ↓
[Main Loop Detects uartDataAvailable()]
   ├── Reads rxByte
   ├── Clears rxDone Flag
   ↓
[Character is Ready for Processing (e.g., Echo)]
   ↓
[End]

What happens when the charcater is echoes back to the PC?

[Start]
   ↓
[Main Loop Detects uartDataAvailable()]
   ├── Calls uartRead()
   ├── Retrieves rxByte
   ├── Clears rxDone Flag
   ↓
[Call uartStartTxDMA(rxByte)]
   ├── Waits for Previous TX Transfer to Complete
   ├── Stores Byte in txByte
   ├── Configures DMA TX Channel 2:
   │   ├── CPAR = UART1->TDR (Peripheral: UART TX Register)
   │   ├── CMAR = &txByte (Memory: Variable to Send)
   │   ├── CNDTR = 1 (Transfer 1 byte)
   │   ├── Sets Direction = Memory to Peripheral
   │   ├── Enables Transfer Complete Interrupt (TCIE)
   │   ├── Enables DMA TX Channel
   ↓
[DMA Transfers txByte to UART1->TDR]
   ↓
[DMA Triggers Transfer Complete Interrupt]
   ↓
[Jump to DMA1_Channel2_3_IRQHandler()]
   ├── Clears TX Transfer Complete Flag
   ├── Sets txDone = true  (Indicates TX is Complete)
   ↓
[Character is Sent to PC Successfully]
   ↓
[End]



--- 

✅ **Key Features:**
- **Bare-metal implementation** (no HAL, but used CMSIS library)
- **DMA-based one-byte UART transmission & reception**
- **Interrupt-driven approach** (efficient CPU usage)
- **Minimal RAM footprint** (no large buffers)
- **Non-blocking TX & RX** using **DMA transfer complete (TC) flags**
- **Supports STM32F072RBT6 and similar Cortex-M0 MCUs**

---

## **Why Use DMA for UART?**
Using **DMA (Direct Memory Access)** with UART eliminates the need for polling or interrupt-driven data handling, reducing CPU load and ensuring efficient data transfer.

| **Method**       | **CPU Overhead** | **Latency** | **Efficiency** | **Best Use Case** |
|------------------|-----------------|-------------|---------------|----------------|
| **Polling**       | High            | High        | ❌ Inefficient  | Simple applications |
| **Interrupts**    | Medium          | Low         | ✅ Efficient    | General-purpose use |
| **DMA**          | Very Low        | Very Low    | ✅✅ **Highly Efficient** | High-speed transfers |

---

## **Implementation Details**
### **1️⃣ UART Initialization (`uartInit`)**
- Configures **TX and RX GPIO pins** as **alternate function**
- Enables **USART1 peripheral clock**
- Sets the **baud rate**
- Enables **DMA mode for TX & RX**
- Configures **DMA1 Channel 2** (TX) and **DMA1 Channel 3** (RX)
- Enables **DMA interrupts** in NVIC

```c
void uartInit(USART_TypeDef *uart, unsigned long baud) {
    RCC->APB2ENR |= BIT(14);  // Enable USART1 clock
    RCC->AHBENR |= BIT(0);    // Enable DMA1 clock

    // Enable DMA mode in USART1 (DMAT for TX, DMAR for RX)
    uart->CR3 |= BIT(7) | BIT(6);

    // Setup DMA1_Channel3 (RX) to receive one byte
    DMA1_Channel3->CPAR = (uint32_t)&UART1->RDR;
    DMA1_Channel3->CMAR = (uint32_t)&rxByte;
    DMA1_Channel3->CNDTR = 1;
    DMA1_Channel3->CCR = BIT(1);  // Enable Transfer Complete Interrupt
    DMA1_Channel3->CCR |= BIT(0); // Enable DMA Channel 3
}
```

---

### **2️⃣ DMA-Based UART Transmission (`uartStartTxDMA`)**
- Waits for the previous transfer to complete
- Loads **txByte** with data
- Configures **DMA1 Channel 2** for TX
- Starts the transfer

```c
void uartStartTxDMA(uint8_t data) {
    while (!txDone && (DMA1_Channel2->CCR & BIT(0))); // Wait for previous TX
    txByte = data;  // Store new byte

    // Setup DMA Channel 2 (TX)
    DMA1_Channel2->CPAR = (uint32_t)&UART1->TDR;
    DMA1_Channel2->CMAR = (uint32_t)&txByte;
    DMA1_Channel2->CNDTR = 1;
    DMA1_Channel2->CCR = BIT(4) | BIT(1);  // Enable memory-to-peripheral mode + TCIE
    DMA1_Channel2->CCR |= BIT(0); // Start DMA transfer
}
```

---

### **3️⃣ DMA Interrupt Handler (`DMA1_Channel2_3_IRQHandler`)**
Handles **both TX and RX DMA transfer completion:**
- **TX Complete:** Clears `txDone` flag
- **RX Complete:** Clears `rxDone` flag, prepares next reception

```c
void DMA1_Channel2_3_IRQHandler(void) {
    if (DMA1->ISR & BIT(5)) {  // TX Complete
        DMA1->IFCR |= BIT(5);  // Clear flag
        txDone = true;
    }

    if (DMA1->ISR & BIT(9)) {  // RX Complete
        DMA1->IFCR |= BIT(9);  // Clear flag
        rxDone = true;

        // Re-enable RX DMA for next byte
        DMA1_Channel3->CCR &= ~BIT(0);
        DMA1_Channel3->CNDTR = 1;
        DMA1_Channel3->CCR |= BIT(0);
    }
}
```

---

## **Hardware Connections**
| **STM32 Pin** | **Function** | **UART Signal** |
|--------------|------------|----------------|
| PA9          | TX         | USART1_TX      |
| PA10         | RX         | USART1_RX      |

✅ **DMA Channels Used:**
- **Channel 2 → USART1 TX**
- **Channel 3 → USART1 RX**

  ![image](https://github.com/user-attachments/assets/9c17358e-e1a1-4546-a760-bbf4086706e9)

From rm0091 reference manual it is observerd that USART1 TX is mapped to channel 2 and USART1 RX is mapped to channel 3

![image](https://github.com/user-attachments/assets/94ecf7b0-2cdb-4589-b5bb-e50df1075f12)

From SYSCFG configuration register 1 (SYSCFG_CFGR1) we see that we can remap USART1 TX RX to different channels but we will use it in default state.(channel 2 and 3).

![image](https://github.com/user-attachments/assets/57bf429e-a92b-47cc-a1cd-b3d119bc6141)

#### **DMA Block Diagram Explanation for STM32F072RBT6**  

The **Direct Memory Access (DMA) controller** in the **STM32F072RBT6** is designed to transfer data between peripherals (e.g., USART, SPI, ADC) and memory **without CPU intervention**. This reduces CPU load, improves performance, and enables real-time data handling.  

The **DMA block diagram** consists of:  
- **DMA Controller (DMA1)**, which has **seven independent channels**, each capable of handling different peripheral requests.  
- Each **DMA channel** connects to a specific peripheral (e.g., Channel 2 for USART1_TX, Channel 3 for USART1_RX).  
- The **DMA request multiplexer** routes peripheral requests to the appropriate DMA channel.  
- The **DMA controller includes a priority system**, allowing high-priority transfers to preempt lower-priority ones.  
- Once a transfer completes, the **DMA interrupt flag is set**, triggering the **NVIC** (Nested Vectored Interrupt Controller) to notify the CPU, enabling efficient event-driven processing.  

By configuring **memory-to-peripheral (TX) and peripheral-to-memory (RX) transfers**, the STM32F0 DMA module enables **low-latency, non-blocking communication**, making it ideal for applications such as **UART DMA-driven data streaming**.

---

## **Demo: How to Use**
1️⃣ **Flash the firmware**
```sh
make jflash or make stflash
```

2️⃣ **Open Minicom (or any serial terminal)**
```sh
minicom -b 115200 -D /dev/ttyUSB0
```

3️⃣ **Send a byte, and see it echoed back!**
```
User Input  → 'A'
Response    → 'A'
```

---

## **Technical Highlights**
✔️ **Zero CPU overhead** (only IRQs execute briefly)  
✔️ **DMA transfer completion flags prevent buffer overflows**  
✔️ **Efficient single-byte handling (ideal for command-based protocols)**  
✔️ **No while-loops blocking execution** (non-blocking approach)  
✔️ **Optimized for minimal power consumption**  

---

## **Potential Enhancements**
🔹 **Support for variable-length packets** (not just single bytes)  
🔹 **Add circular DMA mode for continuous reception**  
🔹 **Implement a ring buffer for TX & RX**  
🔹 **Use FreeRTOS for multitasking (if required)**  

---

## **Conclusion**
This **bare-metal DMA-based UART implementation** is a highly efficient way to handle UART communication on STM32F0 MCUs. The use of **DMA with TX & RX interrupts** ensures **non-blocking, low-latency** operation, making it ideal for real-time applications.

🚀 **Want to Contribute?** Open a Pull Request or raise an Issue!  
📧 **Questions?** Contact me via GitHub Discussions.

