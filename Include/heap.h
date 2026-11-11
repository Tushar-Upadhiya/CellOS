#pragma once
#include<stddef.h>
void* myalloc(size_t size);
void* mycalloc(size_t num, size_t size);
void myfree(void* ptr);