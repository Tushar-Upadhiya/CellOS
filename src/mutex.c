#include "mutex.h"
#include "task.h"

#define ICSR (*(volatile uint32_t *)0xE000ED04)
void mutex_lock(my_mutex_t*m){
    while(1){
        __asm volatile("CPSID i");
        if(m->locked==0){
            m->locked=1;
            __asm volatile("CPSIE i");
            return ;
        }
        
        current_task->state = TASK_BLOCKED;
        m->waiting_task=current_task;
        __asm volatile("CPSIE i");

        ICSR |= (1<<28);
    }
}
void mutex_unlock(my_mutex_t *m) {
    __asm volatile ("CPSID i");
    m->locked = 0;
    if(m->waiting_task!=0){
        m->waiting_task->state = TASK_READY;
        m->waiting_task = 0;
    }
    __asm volatile ("CPSIE i");
}