#pragma once
#include<stdint.h>
typedef struct{
    volatile uint32_t locked;
}my_mutex_t;

void mutex_lock(my_mutex_t* m);
void mutex_unlock(my_mutex_t*m);
