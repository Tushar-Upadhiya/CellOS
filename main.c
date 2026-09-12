#include <stdint.h>
#include "systick.h"
#include "task.h"

#define RCC_AHB1ENR   (*(volatile uint32_t *)0x40023830)
#define GPIOA_MODER   (*(volatile uint32_t *)0x40020000)
#define GPIOA_ODR     (*(volatile uint32_t *)0x40020014)

#define GPIOAEN       (1 << 0)
#define PA0           0
#define PA1           1

void delay(volatile uint32_t count) {
    while (count--) { }
}

void task1_func(void) {
    while (1) {
        GPIOA_ODR ^= (1 << PA0);
        delay(500000);
    }
}

void task2_func(void) {
    while (1) {
        GPIOA_ODR ^= (1 << PA1);
        delay(500000);
    }
}

int main(void) {
    RCC_AHB1ENR |= GPIOAEN;

    GPIOA_MODER &= ~(0b11 << (PA0 * 2));
    GPIOA_MODER |=  (0b01 << (PA0 * 2));
    GPIOA_MODER &= ~(0b11 << (PA1 * 2));
    GPIOA_MODER |=  (0b01 << (PA1 * 2));

    task_create(task1_func);
    task_create(task2_func);

    SysTick_Init();
    os_start();

    while (1) { }   // never actually reached
}