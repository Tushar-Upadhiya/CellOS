#include<stdint.h>
#define RCC_AHB1STR (*((volatile uint32_t*)0x40023810))
#define RCC_AHB1ENR (*((volatile uint32_t*)0x40023830))
#define RCC_APB2ENR (*((volatile uint32_t*)0x40023844))

void reset_GPIOA(void){
    RCC_AHB1STR |=(1U<<0);

    for(int i =0;i<100;i++);

    RCC_AHB1STR &=~(1U<<0);
}

void uart_init(void){

    RCC_AHB1ENR|=(1U<<0);

    RCC_APB2ENR|=(1U<<4);
}
