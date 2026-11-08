#include<stdint.h>

#define RCC_BASE 0x40023800
#define RCC_AHB1ENR (*((volatile uint32_t*)(RCC_BASE + 0x30)))
#define RCC_APB2ENR (*((volatile uint32_t*)(RCC_BASE + 0x44)))

#define GPIOA_BASE 0x40020000
#define GPIOA_MODER (*((volatile uint32_t*)(GPIOA_BASE + 0x00)))
#define GPIOA_OSPEEDR (*((volatile uint32_t*)(GPIOA_BASE +0x08)))
#define GPIOA_AFRL (*((volatile uint32_t*)(GPIOA_BASE + 0x20)))
#define GPIOA_BSRR (*((volatile uint32_t*)(GPIOA_BASE + 0x18)))

#define SPI1_BASE 0x40013000
#define SPI1_CR1 (*((volatile uint32_t*)(SPI1_BASE + 0x00)))
#define SPI1_CR2 (*((volatile uint32_t*)(SPI1_BASE + 0x04)))
#define SPI1_SR (*((volatile uint32_t*)(SPI1_BASE + 0x08)))
#define SPI1_DR (*((volatile uint32_t*)(SPI1_BASE + 0x0C)))

void spi1_init(void) {
    RCC_AHB1ENR |= (1U << 0);
    RCC_APB2ENR |= (1U << 12);

    GPIOA_MODER &= ~((3U << (4 * 2)) | (3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOA_MODER |= ((1U << (4 * 2)) | (2U << (5 * 2)) | (2U << (6 * 2)) | (2U << (7 * 2)));

    GPIOA_OSPEEDR |= ((3U << (4 * 2)) | (3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));

    GPIOA_AFRL &= ~((0xFU << (5 * 4)) | (0xFU << (6 * 4)) | (0xFU << (7 * 4)));
    GPIOA_AFRL |= ((5U << (5 * 4)) | (5U << (6 * 4)) | (5U << (7 * 4)));

    GPIOA_BSRR = (1U << 4);

    SPI1_CR1 = 0;
    SPI1_CR1 |= (1U << 2);
    SPI1_CR1 |= (3U << 3);
    SPI1_CR1 |= (1U << 9) | (1U << 8);
    SPI1_CR1 |= (1U << 6);
}

void spi1_cs_low(void) {
    GPIOA_BSRR = (1U << (4 + 16));
}

void spi1_cs_high(void) {
    GPIOA_BSRR = (1U << 4);
}

uint8_t spi1_transfer(uint8_t data) {
    while (!(SPI1_SR & (1U << 1)));
    SPI1_DR = data;
    while (!(SPI1_SR & (1U << 0)));
    while (SPI1_SR & (1U << 7));
    return SPI1_DR;
}
