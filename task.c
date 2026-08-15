#include "task.h"

TCB tasks[MAX_TASKS];
uint32_t task_stacks[MAX_TASKS][STACK_SIZE];
uint32_t task_count = 0;

void task_return_handler(void) {
    while (1) { }
}

void task_create(void (*task_func)(void)) {
    uint32_t id = task_count;
    task_count++;

    uint32_t *sp = &task_stacks[id][STACK_SIZE];

    *(--sp) = 0x01000000;
    *(--sp) = (uint32_t)task_func;
    *(--sp) = (uint32_t)task_return_handler;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;

    tasks[id].stack_pointer = sp;
    tasks[id].state = TASK_READY;
    tasks[id].task_id = id;
}