#include "hal.h"

/*
STEPS:
1. Configure GPIOs to have alternate function
2. Configure UART
3. In ISR do operations
 */

void spin(volatile uint32_t count) {
	while (count--) (void) 0;
}

void gpio_set_mode(uint16_t pin, uint8_t mode) {
	GPIO_TypeDef *gpio = GPIO(PINBANK(pin));
	int n = PINNO(pin);
	RCC->AHBENR |= BIT(17 + PINBANK(pin)); // Enable GPIO clock
	gpio->MODER &= ~(3U << (n * 2));
	gpio->MODER |= (mode & 3U) << (n * 2);
}

void gpioSetAF(uint16_t pin, uint8_t afNum) {
	GPIO_TypeDef *gpio = GPIO(PINBANK(pin));
	int n = PINNO(pin);

	if (n < 8) {
		gpio->AFR[0] &= ~(0xFUL << (n * 4));
		gpio->AFR[0] |= ((uint32_t)afNum << (n * 4));
	} else {
		gpio->AFR[1] &= ~(0xFUL << ((n - 8) * 4));
		gpio->AFR[1] |= ((uint32_t)afNum << ((n - 8) * 4));
	}
}

void gpio_write(uint16_t pin, bool val) {
	GPIO_TypeDef *gpio = GPIO(PINBANK(pin));
	gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

