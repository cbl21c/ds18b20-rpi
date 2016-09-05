/*
** Library to drive a multiplexed 4-digit 7-segment LED display
** Display must be common cathode as the segments are driven HIGH to activate
**
** Two pins are used to select the digit to illuminate. These pins should
** feed into the inputs of a 2-to-4 decoder. The decoder can be active HIGH
** or active LOW, with the appropriate BJT (NPN or PNP respectively) to turn
** the LEDs on.
**
** F_CPU must be defined with the clock speed
**
** As this circuit requires 9 pins, the ATtiny25/45/85 variants do not have enough
** pins and are not supported. Actual pin configuration depends on MCU.
**
** ATtiny84 pin configuration is:
**   PA0 - segment A
**   PA1 - segment B
**   PA2 - segment C
**   PA3 - segment D
**   PA4 - segment E
**   PA5 - segment F
**   PA6 - segment G
**   PA7 - segment DP
**
**   PB0 - Digit Select 1 (DS1)
**   PB1 - Digit Select 2 (DS2)
**
** ATmega328 pin configuration is:
**   PB0 - segment A
**   PB1 - segment B
**   PB2 - segment C
**   PB3 - segment D
**   PB4 - segment E
**   PB5 - segment F
**   PB6 - segment G
**   PB7 - segment DP
**
**   PC0 - Digit Select 1 (DS1)
**   PC1 - Digit Select 2 (DS2)
*/

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

#define TRUE 1
#define FALSE 0

#define DISPLAY_MAX 9999
#define DISPLAY_MIN -999

/* ATtiny84 - LED port = A, DS port = B */
#ifdef _AVR_IOTN84_H_
#define LEDPORT	PORTA
#define DSPORT	PORTB
#endif /* _AVR_IOTN84_H_ */

/* ATmega328 - LED port = B, DS port = C */
#ifdef _AVR_IOM328P_H_
#define LEDPORT	PORTB
#define DSPORT	PORTC
#endif /* _AVR_IOM328P_H_ */


/* module variables */
const uint8_t SEGMENTS[17] = 
{
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};

uint8_t digitmap[4];
uint8_t ndigits;


void display_initialise(void)
{
    /* configure LED port for output on all pins */
    LEDPORT = 0xff;
    /* configure DS port for output on pins 0 and 1 */
    DSPORT |= 0x03;
}


int display(void)
{
/*
** displays the characters in the view array
** the function should run an infinite loop so the data to be displayed
** should be configured in the global variables
** the global variables are updated by the interrupt service routine
*/
    uint8_t ds;
    uint8_t dslevel;

    while (1)
        for (ds = 3; ds >= 4 - ndigits; ds--)
        {
            /* select the digit */
            dslevel = (DSPORT & 0xfc) | ds;
            DSPORT = dslevel;

            /* activate the required segments */
            LEDPORT = digitmap[ds];

            /* hold for 2 ms */
            _delay_ms(2);
        }

    return 0;
}


int display10(int number, uint8_t dp)
{
/*
** updates the digitmap array with the values required to display a given number
**
** dp is the number of decimal places (range: 0 to 3)
** number should be expressed in multiples of 10^(-dp)
**
** for example, to display 10.5, number=105 and dp=1
** the largest number that can be displayed is 9999
** the smallest number that can be displayed is -999
** with the relevant decimal places
**
** if the given number is outside this range, then HI or LO will be displayed
** as appropriate
**
** if dp > 3, Err will be displayed
** if dp == 3 and number < 0, Err will be displayed because the number cannot
** be displayed with the requested precision
*/

    int absnum;
    uint8_t dig;
    uint8_t nchars;
    uint8_t minchars;

    if (dp > 3 || ((dp == 3 ) && (number < 0)))
    {
        /* segment levels for "Err" */
        digitmap[3] = 0x05;
        digitmap[2] = 0x05;
        digitmap[1] = 0x79;
        ndigits = 3;
        return 0;
    }

    if (number < DISPLAY_MIN)
    {
        /* segment levels for "LO" */
        digitmap[3] = 0x3f;
        digitmap[2] = 0x38;
        ndigits = 2;
        return 0;
    }

    if (number > DISPLAY_MAX)
    {
        /* segment levels for "HI" */
        digitmap[3] = 0x30;
        digitmap[2] = 0x76;
        ndigits = 2;
        return 0;
    }

    /* if abs(number) < 1, need to fill in leading zeros */
    minchars = dp + 1;

    /* take the absolute value of number so that we don't get -ve modulo */
    absnum = (number < 0 ? -number : number);
    nchars = 0;
    while (absnum > 0 || nchars < minchars)
    {
        dig = absnum % 10;
        digitmap[3 - nchars] = SEGMENTS[dig];
        nchars++;
        absnum = absnum / 10;
    }

    /* add the decimal point */
    if (dp > 0)
        digitmap[3 - dp] |= 0x80;

    /* add the negative sign */
    if (number < 0)
    {
        digitmap[3 - nchars] = 0x40;
        nchars++;
    }

    /* update global variables */
    ndigits = nchars;

    return 0;
}


int display16(uint16_t number, uint8_t leadzeros)
{
/*
** updates the digitmap array with the values required to display a given hex number
** this is a lot simpler than decimal as we don't need to worry about decimal points
** and negative numbers
**
** number is the hex number to be displayed
** leadzeros flags whether leading zeros should be displayed
**
*/
    uint8_t ds;
    uint16_t hex;
    uint8_t dig;
    uint8_t nchars;

    nchars = 0;
    hex = number;
    while (hex > 0)
    {
        dig = hex % 16;
        digitmap[4 - nchars] = SEGMENTS[dig];
        nchars++;
        hex = hex / 16;
    }

    /* if number is zero, we need to show at least a single zero */
    if (number == 0)
    {
        digitmap[3] = SEGMENTS[0];
        nchars = 1;
    }

    for (ds = 0; (ds < 4 - nchars) && leadzeros; ds++)
    {
        digitmap[ds] = SEGMENTS[0];
        nchars++;
    }

    ndigits = nchars;

    return 0;
}

