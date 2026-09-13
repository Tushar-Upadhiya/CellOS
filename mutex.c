#include "mutex.h"
void mutex_lock(my_mutex_t*m){
    while(1){
        __asm volatile("CPSID i");
        if(m->locked==0){
            m->locked=1;
            __asm volatile("CPSIE i");
            return ;
        }
        __asm volatile("CPSIE i");
    }
}
void mutex_unlock(my_mutex_t *m) {
    __asm volatile ("CPSID i");
    m->locked = 0;
    __asm volatile ("CPSIE i");
}