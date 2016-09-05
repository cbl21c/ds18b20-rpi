#define F_CPU 1000000L
#include <avr/io.h>
#include <util/delay.h>
#include <sensor/ds18b20.h>
#include "display.h"

#define DELAY_LOOPS 125

/* 1 decimal point values for 1/16th fraction */
const uint8_t rounding[16] = {0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 9};

int main(void)
{
    uint8_t   n;
    int16_t   temp16;
    int16_t   temp10;
    int8_t    sign;
    int8_t    tempint;
    int8_t    tempfrac;
    uint8_t   found;
    ds18b20_t dev;

    /* set PA as all output */
    DDRA = 0xff;

    /* set up PB2 as DS18B20 DQ */
    dev.port = DS_PORT_B;
    dev.pin = 2;

    found = ds18b20_init(&dev);

    /* PA7 LED flashing if DS18B20 not detected */
    while (found == 0)
        PORTA ^= 0x80;

    /* if DS18B20 detected, read temperature approx every second and display on LEDs */
    while (1)
    {
        temp16 = ds18b20_read_temperature(&dev);

        sign = (temp16 < 0 ? -1 : 1);
        tempint = temp16 / 16;
        tempfrac = sign * (temp16 % 16);

        temp10 = tempint / 16 * 10;
        temp10 += rounding[tempfrac];

        for (n = 0; n < DELAY_LOOPS; n++)
            display10(temp10, 1);
    }

    return 0;
}

