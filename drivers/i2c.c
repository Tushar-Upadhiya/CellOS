#include<stdint.h>

#define RCC_BASE 0x40023800
#define RCC_AHB1ENR (*((volatile uint32_t*)(RCC_BASE + 0x30)))
#define RCC_APB1ENR (*((volatile uint32_t*)(RCC_BASE + 0x40)))

#define GPIOB_BASE 0x40020400
#define GPIOB_MODER (*((volatile uint32_t*)(GPIOB_BASE+0x00)))
#define GPIOB_PUPDR (*((volatile uint32_t*)(GPIOB_BASE + 0x0C)))
#define GPIOB_OTYPER (*((volatile uint32_t*)(GPIOB_BASE + 0x04)))
#define GPIOB_AFRL (*((volatile uint32_t*)(GPIOB_BASE + 0x20)))

#define I2C1_BASE 0x40005400
#define I2C_CR1 (*((volatile uint32_t*)(I2C1_BASE + 0x00)))
#define I2C_CR2 (*((volatile uint32_t*)(I2C1_BASE + 0x04)))
#define I2C_SR1 (*((volatile uint32_t*)(I2C1_BASE + 0x14)))
#define I2C_SR2 (*((volatile uint32_t*)(I2C1_BASE + 0x18)))
#define I2C_CCR (*((volatile uint32_t*)(I2C1_BASE + 0x1C)))
#define I2C_DR  (*((volatile uint32_t*)(I2C1_BASE + 0x10)))
#define I2C_TRISE (*((volatile uint32_t*)(I2C1_BASE + 0x20)))

void i2c1_init(void){
    RCC_AHB1ENR |= (1U<<1);
    RCC_APB1ENR |= (1U<<21);

    GPIOB_MODER &= ~((3U<<(6*2))|(3U<<(7*2)));
    GPIOB_MODER |= ((2U<<(6*2))|(2U<<(7*2)));

    GPIOB_OTYPER |= ((1U<<6)|(1U<<7));

    GPIOB_PUPDR &= ~((3U<<(6*2))|(3U<<(7*2)));
    GPIOB_PUPDR |= ((1U<<(6*2))|(1U<<(7*2)));

    GPIOB_AFRL &= ~((0xFU << (6*4))|(0xFU << (7*4)));
    GPIOB_AFRL |= ((4U<<(6*4))|(4U<<(7*4)));

    I2C_CR1 &= ~(1U<<0);
    I2C_CR2 = 16;
    I2C_CCR = 80;
    I2C_TRISE = 17; 
    I2C_CR1 |= (1U<<0);
}

void i2c1_write_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t data){
    while(I2C_CR1&(1U<<15));
    I2C_CR1 |= (1U<<8);

    while(!(I2C_SR1 & (1U<<0)));
    I2C_DR = (slave_addr<<1);

    while(!(I2C_SR1&(1U<<1)));
    (void)I2C_SR1;
    (void)I2C_SR2;

    while (!(I2C_SR1 & (1U << 7)));
    I2C_DR = reg_addr;

    while (!(I2C_SR1 & (1U << 7)));
    I2C_DR = data;

    while (!(I2C_SR1 & (1U << 7)) || !(I2C_SR1 & (1U << 2)));
    I2C_CR1 |= (1U << 9);
}

uint8_t i2c1_read_byte(uint8_t slave_addr, uint8_t reg_addr) {
    while (I2C_CR1 & (1U << 15));
    
    I2C_CR1 |= (1U << 8);
    while (!(I2C_SR1 & (1U << 0)));

    I2C_DR = (slave_addr << 1);
    while (!(I2C_SR1 & (1U << 1)));
    (void)I2C_SR1;
    (void)I2C_SR2;

    while (!(I2C_SR1 & (1U << 7)));
    I2C_DR = reg_addr;
    while (!(I2C_SR1 & (1U << 7)));

    I2C_CR1 |= (1U << 8);
    while (!(I2C_SR1 & (1U << 0)));

    I2C_DR = (slave_addr << 1) | 1U;
    while (!(I2C_SR1 & (1U << 1)));

    I2C_CR1 &= ~(1U << 10);
    I2C_CR1 |= (1U << 9);

    (void)I2C_SR1;
    (void)I2C_SR2;

    while (!(I2C_SR1 & (1U << 6)));
    uint8_t data = I2C_DR;
    
    I2C_CR1 |= (1U << 10);
    
    return data;
}