#include <stdint.h>

#define RCC_AHB1ENR   (*(volatile uint32_t *)0x40023830)
#define GPIOA_MODER   (*(volatile uint32_t *)0x40020000)
#define GPIOA_ODR     (*(volatile uint32_t *)0x40020014)

#define GPIOAEN       (1 << 0)   // bit 0 in AHB1ENR enables GPIOA's clock
#define PA0           0

void delay(volatile uint32_t count) {
    while (count--) { }
}

int main(void) {
    RCC_AHB1ENR |= GPIOAEN;

    GPIOA_MODER &= ~(0b11 << (PA0 * 2));
    GPIOA_MODER |=  (0b01 << (PA0 * 2));

    while (1) {
        GPIOA_ODR ^= (1 << PA0);
        delay(500000);
    }

    return 0;
}