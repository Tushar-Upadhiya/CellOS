#include<stdint.h>

extern uint32_t _estack;

void Reset_Handler(void);

void Default_Handler(void){
    while(1){

    }
}

void NMI_Handler(void) __attribute__((weak,alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak,alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak,alias("Default_Handler")));

__attribute__((section(".isr_vector")))
void(*const vector_table[])(void)={
    (void(*)(void))&_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,

};