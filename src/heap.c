#include <stdint.h>
#include <stdbool.h>
#include "heap.h"

typedef struct header{
    size_t size;
    bool isfree;
    header* next;
}header;

extern uint8_t __heap_start[];
extern uint8_t __heap_end[];

static void* heap_start = NULL;

void heap_init(void){

}

void* myalloc(size_t size){
    if(size==0) return NULL;
    if(heap_start==NULL){
        heap_start = sbrk(0);
    }
    size_t aligned_size = (size + sizeof(header) + 7) & ~7;
}

void myfree(void* ptr){
    if(ptr==NULL) return;

}