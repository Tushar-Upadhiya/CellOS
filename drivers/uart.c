#include<stdint.h>

#define RCC_AHB1STR (*((volatile uint32_t*)0x40023810))
#define RCC_AHB1ENR (*((volatile uint32_t*)0x40023830))
#define RCC_APB2ENR (*((volatile uint32_t*)0x40023844))

#define GPIOA_BASE  0x40020000
#define GPIOA_MODER  (*((volatile uint32_t*)(GPIOA_BASE+0x00)))
#define GPIOA_AFRH  (*((volatile uint32_t*)(GPIOA_BASE+0x24)))

#define USART1_BASE  0x40011000
#define USART1_BRR  (*((volatile uint32_t*)(USART1_BASE+0x08)))
#define USART_CR1   (*((volatile uint32_t*)(USART1_BASE+0x0C)))
#define USART1_SR   (*((volatile uint32_t*)(USART1_BASE+0x00)))
#define USART1_DR   (*((volatile uint32_t*)(USART1_BASE+0x04)))


void reset_GPIOA(void){
    RCC_AHB1STR |=(1U<<0);

    for(volatile int i =0;i<100;i++);

    RCC_AHB1STR &=~(1U<<0);
}

void uart_init(void){

    RCC_AHB1ENR|=(1U<<0);

    RCC_APB2ENR|=(1U<<4);
}

void gpio_init(void){
    //configuring PA9 and PA10 to alternate function mode (Binary '10'=2)
    //each pin takes 2 bits in moder
    //PA9 uses bits [19:18] , PA10 uses bits [21:20]

    //clearing old bits
    GPIOA_MODER&= ~((3U<<(9*2))| (3U<<(10*2)));;

    //set both to Alternate function mode
    GPIOA_MODER|= ((2U<<(9*2))|(2U<<(10*2)));

    //configure alternate function high register(AFRH) for (Binary '0111' = 7)  
    //AFRH handles pin 8 to pin 15
    //PA9 is AFRH9 (bits [7:4]), PA10 is AFRH10 (bits [11:8])

    //clear old bits from PA9 to PA10 in AFRH
    GPIOA_AFRH &= ~((0xFU << (4 * (9 - 8))) | (0xFU << (4 * (10 - 8))));
    // Set both to AF7 (7)
    GPIOA_AFRH |=  ((7U  << (4 * (9 - 8))) | (7U  << (4 * (10 - 8))));
}

void uart_set_baudrate(void){
    //for 16MMHz clock and 9600 baud
    //USARTDIV = 16000000/(16*9600)=104.1667
    //Mantissa 104 (0x68), Fraction=3 -> 0x683

    USART1_BRR = 0x683;
}

void uart_enable(void){
    //13th bit -> UART ENABLE (UE), 3rd bit -> TRANSMITTER ENABLE (TE), 2nd bit -> RECEIVER ENABLE (RE)
    USART_CR1|=(1U<<13)|(1U<<3)|(1U<<2);
}

void uart_writechar(char c){
    while(!(USART1_SR & (1U<<7)));
    USART1_DR = c;
}

void uart_writestring(const char *str) {
    while (*str != '\0') {
        uart_writechar(*str);
        str++;
    }
}
