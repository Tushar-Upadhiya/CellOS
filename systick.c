#include<stdint.h>
#include "systick.h"

#define SYST_CSR   (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR   (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR   (*(volatile uint32_t *)0xE000E018)

volatile uint32_t tick_count = 0;

void SysTick_Init(void) {
    SYST_RVR = 15999;
    SYST_CVR = 0;
    SYST_CSR = (1 << 0) | (1 << 1) | (1 << 2);
}

void SysTick_Handler(void){
    tick_count++;
}
