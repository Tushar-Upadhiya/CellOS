#include <stdint.h>
#include "task.h"

void reset_GPIOA(void);
void uart_init(void);
int main(void) {
reset_GPIOA();

uart_init();

    while (1) { }   // never actually reached
}