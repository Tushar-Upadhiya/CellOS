#include<stdint.h>
#include"systick.h"

#define RCC_AHB1ENR   (*(volatile uint32_t *)0x40023830)
#define GPIOA_MODER   (*(volatile uint32_t *)0x40020000)
#define GPIOA_ODR     (*(volatile uint32_t *)0x40020014)

#define GPIOAEN       (1 << 0)
#define PA0           0

int main(void) {
    RCC_AHB1ENR |= GPIOAEN;

    GPIOA_MODER &= ~(0b11 << (PA0 * 2));
    GPIOA_MODER |=  (0b01 << (PA0 * 2));

    SysTick_Init();

    while (1) {
        
    }

    return 0;
}