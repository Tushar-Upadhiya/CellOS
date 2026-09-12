#pragma once
#include <stdint.h>
#define MAX_TASKS 4
#define STACK_SIZE 256

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED
} task_state_t;

typedef struct {
    uint32_t * stack_pointer;
    task_state_t state;
    uint32_t task_id;
}TCB;



extern TCB *current_task;
extern TCB *next_task;
extern uint32_t task_count;
void os_start(void);
void task_create(void (*task_func)(void));




extern TCB tasks[MAX_TASKS];
extern uint32_t task_stacks[MAX_TASKS][STACK_SIZE];