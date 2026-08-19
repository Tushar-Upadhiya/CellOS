#include "task.h"

TCB tasks[MAX_TASKS];
uint32_t task_stacks[MAX_TASKS][STACK_SIZE];
uint32_t task_count = 0;
TCB *current_task;
TCB *next_task;

void task_return_handler(void) {
    while (1) { }
}

void task_create(void (*task_func)(void)) {
    uint32_t id = task_count;
    task_count++;

    uint32_t *sp = &task_stacks[id][STACK_SIZE];

    // hardware-auto-stacked frame (unchanged from before)
    *(--sp) = 0x01000000;
    *(--sp) = (uint32_t)task_func;
    *(--sp) = (uint32_t)task_return_handler;
    *(--sp) = 0;   // R12
    *(--sp) = 0;   // R3
    *(--sp) = 0;   // R2
    *(--sp) = 0;   // R1
    *(--sp) = 0;   // R0

    // NEW — software-saved frame, matching what PendSV will pop
    *(--sp) = 0;   // R11
    *(--sp) = 0;   // R10
    *(--sp) = 0;   // R9
    *(--sp) = 0;   // R8
    *(--sp) = 0;   // R7
    *(--sp) = 0;   // R6
    *(--sp) = 0;   // R5
    *(--sp) = 0;   // R4

    tasks[id].stack_pointer = sp;
    tasks[id].state = TASK_READY;
    tasks[id].task_id = id;
}