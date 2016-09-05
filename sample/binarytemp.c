#define F_CPU 1000000L
#include <avr/io.h>
#include <util/delay.h>
#include <sensor/ds18b20.h>

int main(void)
{
    int16_t   temp16;
    int8_t    sign;
    int8_t    tempint;
    int8_t    tempfrac;
    uint8_t   found;
    uint8_t   err;
    ds18b20_t dev;

    /* set PA as all output */
    DDRA = 0xff;

    /* set up PB2 as DS18B20 DQ */
    dev.port = DS_PORT_B;
    dev.pin = 2;

    found = ds18b20_init(&dev);
    err = 0x80;

    /* PA7 LED flashing if incorrect config */
    while (found > 1)
    {
        PORTA ^= err;
        _delay_ms(250);
    }

    /* PA7 LED on if DS18B20 not detected */
    while (found == 0)
        PORTA = err;

    /* if DS18B20 detected, read temperature every second and display on LEDs */
    while (1)
    {
        temp16 = ds18b20_read_temperature(&dev);

        /* check the fractional part so that we do proper rounding */
        sign = (temp16 < 0 ? -1 : 1);
        tempint = temp16 / 16;
        tempfrac = sign * (temp16 % 16);

        if (tempfrac >= 8)
            tempint = sign * (tempint + 1);

        PORTA = dev.tempint;
        _delay_ms(1000);
    }

    return 0;
}

