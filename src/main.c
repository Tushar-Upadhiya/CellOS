#include <stdint.h>

void uart_init(void);
void uart_writestring(const char *str);
void i2c1_init(void);
void i2c1_write_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t data);
uint8_t i2c1_read_byte(uint8_t slave_addr, uint8_t reg_addr);

int main(void) {
    uart_init();

    // MASSIVE delay to let the TTL adapter and picocom wake up
    for (volatile int i = 0; i < 4000000; i++);

    uart_writestring("\r\n\r\n=== SYSTEM BOOTING ===\r\n");

    i2c1_init();
    uart_writestring("[1] I2C Peripheral Initialized.\r\n");

    // Small delay before trying to talk to the sensor
    for (volatile int i = 0; i < 1000000; i++);

    uart_writestring("[2] Attempting to wake up MPU6050 (Sending Write)...\r\n");
    
    // THIS is likely where it freezes!
    i2c1_write_byte(0x68, 0x6B, 0x00);
    
    uart_writestring("[3] Write successful! Sensor acknowledged.\r\n");

    while (1) {
        uint8_t who_am_i = i2c1_read_byte(0x68, 0x75);

        if (who_am_i == 0x68) {
            uart_writestring("MPU6050 Connected! WHO_AM_I = 0x68\r\n");
        } else {
            uart_writestring("Sensor read, but wrong ID!\r\n");
        }

        for (volatile int i = 0; i < 2000000; i++);
    }
}