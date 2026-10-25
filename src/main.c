#include <stdint.h>

void reset_GPIOA(void);
void uart_init(void);
void gpio_init(void);
void uart_set_baudrate(void);
void uart_enable(void);
void uart_writestring(const char *str);

int main(void) {
    reset_GPIOA();
    uart_init();
    gpio_init();
    uart_set_baudrate();
    uart_enable();

    while (1) {
        uart_writestring("Hello from CellOS running on STM32F401!\r\n");
        for (volatile int i = 0; i < 500000; i++); // Delay between messages
    }
}