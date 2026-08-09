#include<stdint.h>

extern uint32_t _estack;

void Reset_Handler(void);

void Default_Handler(void){
    while(1){

    }
}

// Core Cortex-M4 Processor Exceptions Handlers
void NMI_Handler(void) __attribute__((weak,alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak,alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak,alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak,alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak,alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak,alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak,alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak,alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak,alias("Default_Handler")));

__attribute__((section(".isr_vector")))
void(*const vector_table[])(void)={
    (void(*)(void))&_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0, //here all 0's represent reserved slots in the vector table
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,

};