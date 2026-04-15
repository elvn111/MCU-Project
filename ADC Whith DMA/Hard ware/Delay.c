#include "stm32f10x.h"

void Delay_us(uint16_t us)
{
    uint32_t count = 0;
    while (us--) {
        count = 72; // Assuming a 72 MHz clock, this will give approximately 1 microsecond delay
        while (count--);
    }
}

void Delay_ms(uint16_t ms)
{
    uint32_t count = 0;
    while (ms--) {
        count = 72000; // Assuming a 72 MHz clock, this will give approximately 1 millisecond delay
        while (count--);
    }
}

void Delay_s(uint16_t s)
{
    while (s--) {
        Delay_ms(1000); // Call the millisecond delay function to achieve a 1 second delay
    }
}