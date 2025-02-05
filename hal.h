#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h> 
#include <stdlib.h>

#include "stm32f072xb.h"

#define FREQ 8000000  // HSI clock frequency is 8 MHz by default
#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank)-'A')<<8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)
#define GPIO(bank) ((GPIO_TypeDef *) (GPIOA_BASE + 0x400U * (bank)))

enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };

#define RCC ((RCC_TypeDef *) RCC_BASE)
#define UART1 USART1

void spin(volatile uint32_t count);
void gpio_set_mode(uint16_t pin, uint8_t mode);
void gpioSetAF(uint16_t pin, uint8_t afNum);
void gpio_write(uint16_t pin, bool val);

