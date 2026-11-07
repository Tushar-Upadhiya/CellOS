#include<stdint.h>

#define RCC_BASE 0x40023800
#define RCC_AHB1ENR (*((volatile uint32_t*)(RCC_BASE + 0x30)))
#define RCC_APB2ENR (*((volatile uint32_t*)(RCC_BASE + 0x44)))