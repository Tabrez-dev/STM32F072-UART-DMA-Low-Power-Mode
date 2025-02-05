
# STM32-UART-Interrupt-Based-Driver

This repository implements a UART device driver for STM32F072RBT6-DISCO using interrupt-based methods for both transmit (TX) and receive (RX) operations. The driver is optimized using circular buffers to manage data efficiently and prevent data loss. It focuses on performance and reliability, making it suitable for embedded systems that require UART communication.

<details>
  <summary><h2>Features</h2></summary>
  
- **Interrupt-based UART communication** for both TX and RX.
- **Circular buffer management** for efficient handling of data, reducing overhead and preventing buffer overflow.
- **GPIO initialization for Alternate Function (AF) mode** to configure UART pins (TX/RX).
- Configurable baud rate and other UART settings.
- **Modular structure** with clear separation of concerns:
  - `hal.c` and `hal.h`: Low-level hardware abstraction layer for STM32 peripherals.
  - `uart.c` and `uart.h`: UART-specific driver functions.
  - `main.c`: Application logic for managing UART communication.
  
</details>

<details>
  <summary><h2>Technical Details</h2></summary>

### Circular Buffers for RX and TX

In this project, we use **circular buffers** to manage the RX and TX data. Circular buffers are efficient data structures that allow for the continuous storage and retrieval of data in a fixed-size buffer without the need for resizing or shifting data. 

#### **RX Buffer**
The RX buffer stores incoming UART data that is received through interrupts. When the `RXNE` (Receive Not Empty) interrupt occurs, the received byte is placed in the buffer at the position pointed by `rxHead`. The `rxTail` pointer tracks the location of the oldest unread byte. 

I ensure that the buffer does not overflow by checking the position of `rxHead` and `rxTail` before writing to the buffer. If the buffer is full, I discard the new data to prevent overwriting. This approach ensures that the system remains responsive and prevents data loss under high data rate conditions.

#### **TX Buffer**
Similarly, the TX buffer stores outgoing UART data that will be transmitted. Data is written to the buffer at the position pointed by `txHead`, and `txTail` points to the oldest byte that is ready for transmission. The `TXE` (Transmit Empty) interrupt is used to trigger the sending of data when the UART is ready for the next byte.

### Advantages of Interrupt-based Communication

The **interrupt-based approach** provides several advantages:
- **Reduced CPU Usage**: Interrupts allow the CPU to perform other tasks until data is available or the UART is ready for transmission. This is more efficient than polling, where the CPU continuously checks for the status.
- **Low Latency**: The interrupt handler is triggered immediately when data is received or when the UART is ready to transmit. This minimizes the delay between receiving and processing data.
- **Scalability**: The interrupt-driven approach scales well for high-speed data communication as it doesn't require constant polling, freeing up CPU time for other operations.

By using interrupts for both RX and TX, we ensure that data is handled efficiently without wasting CPU cycles or introducing unnecessary delays.

Here’s the updated section for **GPIO Initialization for AF Mode** based on your code:


### GPIO Initialization for AF Mode

The UART communication relies on specific GPIO pins configured for **Alternate Function (AF) mode**. The following steps are taken to initialize the GPIO registers for UART functionality:

1. **Enable GPIO Clock**: The clock for the GPIO port must be enabled. For UART1, this is done by setting the appropriate bit in the `RCC->AHB1ENR` register. The function `gpio_set_mode` ensures that the GPIO clock for the corresponding pin is enabled and its mode is set to the required mode for UART. Specifically, the `RCC->AHBENR |= BIT(17 + PINBANK(pin));` line enables the GPIO clock for the specified pin bank.

2. **Set GPIO Pin to Alternate Function Mode**: The function `gpioSetAF` configures the GPIO pin to the correct alternate function mode required for UART. It writes the appropriate Alternate Function (AF) number to the GPIO's AFR (Alternate Function Register). Depending on whether the pin number is less than 8 or greater than or equal to 8, the corresponding half of the `AFR` register is modified to select the correct AF function. This allows the UART signal to be routed through the selected pin.

![image](https://github.com/user-attachments/assets/8b1593e6-a257-43dd-aa51-d1f31501fb63)

PA9 and PA10 were used in AF1 mode according to the mapping provided in this document: https://www.st.com/resource/en/datasheet/stm32f072rb.pdf

### UART Initialization

The `uartInit` function configures the UART peripheral with the necessary settings for communication. The following steps outline the process of initializing the UART:

1. **Enable UART Clock**: The clock for the selected UART peripheral is enabled by modifying the `RCC->APB2ENR` register. In this case, for UART1, the appropriate bit is set to enable the clock.

2. **Set GPIO Pins for TX/RX**: The TX (Transmit) and RX (Receive) pins are determined based on the selected UART. For UART1, these are pins PA9 (TX) and PA10 (RX). The `gpio_set_mode` function is used to configure these pins for Alternate Function (AF) mode. Additionally, the `gpioSetAF` function assigns the correct alternate function to the pins, allowing them to function as UART TX and RX.

3. **Configure UART Settings**: The UART configuration is set up in the `CR1` register of the selected UART. First, the register is cleared to ensure a known state. The baud rate is configured by setting the `BRR` (Baud Rate Register) to a value based on the system clock frequency and the desired baud rate.

4. **Enable UART**: The UART transmission and reception are enabled by setting the appropriate bits in the `CR1` register. Specifically, the `RE` (Receiver Enable) and `TE` (Transmitter Enable) bits are set to allow data reception and transmission. 

5. **Enable RXNE Interrupt**: The `RXNE` (Receive Not Empty) interrupt is enabled by setting the corresponding bit in the `CR1` register. This triggers an interrupt whenever new data is received on the UART.

6. **Enable UART Interrupt in NVIC**: The interrupt service routine for UART1 is enabled in the NVIC (Nested Vectored Interrupt Controller). This ensures that the appropriate interrupt handler will be called when an interrupt occurs (e.g., when data is received).

This process configures the UART for communication, ensuring that the TX and RX pins are properly set for alternate function mode and that the UART interrupts are enabled for efficient data transfer.

</details>

<details>
  <summary><h2>How to build and run</h2></summary>

1. **Install Prerequisites**: Ensure that you have the necessary tools installed for ARM development. This includes `arm-none-eabi-gcc`, `make`, and other related tools.

2. **Build the Project**:
    To compile and generate the firmware, run the following command:
    ```bash
    make build
    ```


3. **Flash the Firmware**: Use a suitable programmer, such as **ST-Link** or **J-Link**, and the respective tool to flash the firmware onto your STM32 microcontroller. Make sure you have **ST-LINk** or **J-LINK** Utility installed on your device.

    - For **ST-Link**, use:
        ```bash
        make stflash
        ```

    - For **J-Link**, use:
        ```bash
        make jflash
        ```

5. **Monitor the Output**: After flashing the firmware, use a serial terminal program like **Minicom** to view the UART output from your STM32 microcontroller.

The data is transimitted to the terminal after recieving 5 bytes of data.

![Screenshot from 2025-02-02 07-03-27](https://github.com/user-attachments/assets/b8a8370e-8512-4191-8d7f-6b12c11c4488)

</details>

<details>
  <summary><h2>Makefile Details</h2></summary>
  
- **CFLAGS**: This sets the compiler flags for the build. It includes optimization settings, warning flags, debugging options, and include paths.
  
- **LDFLAGS**: This specifies the linker flags. It includes the linker script, the library options, and optimization for unused sections.

- **Build Targets**:
  - `build`: This target is used to build the firmware from the source files.
  - `firmware.bin`: Converts the compiled ELF file into a binary file (`firmware.bin`) that can be flashed to the STM32.
  - `firmware.elf`: The output ELF file generated by the build process.

- **Flashing**:
  - **stflash**: Flash the firmware using **ST-Link** and reset the microcontroller.
  - **jflash**: Flash the firmware using **J-Link**. The flashing process is controlled by a J-Link script file (`jflash.script`).

### Additional Information:

- **`cmsis_core`**: The CMSIS core files are fetched from the ARM repository.
- **`cmsis_f0`**: The CMSIS device files for STM32F0 series are fetched from the STM32 repository.

### Clean Project:

To clean up all generated files (such as ELF, binary, and script files), run:
```bash
make clean
```
</details>

<details>
  <summary><h2>DIY</h2></summary>
  
To use `printf` for UART output instead of directly writing data byte-by-byte, you can modify the `syscalls.c` file to implement the `_write` function. This function is called by the standard library when `printf` is used. In the provided code, the `_write` function is configured to call `uartWriteBuf` to transmit data. If you wish to use `printf`, simply ensure that the `syscalls.c` file is properly included in your project, and `uartWriteBuf` is correctly implemented to send data over UART. By doing this, you can use the standard `printf` function for formatted output, which will internally call your UART driver to transmit the data over UART.
  
</details>
