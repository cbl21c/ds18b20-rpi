/*
** Dallas DS18B20 Temperature Sensor Library (Raspberry Pi)
**
** This is a library to access the Dallas DS18B20 temperature sensor.
** The functions contained within this library implement the Dallas 1-Wire protocol.
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "gpio.h"
#include "ds18b20.h"
#include "crc1w.h"

#define FALSE 0
#define TRUE  1

/* ROM commands */
#define SEARCH_ROM      0xf0
#define READ_ROM        0x33
#define MATCH_ROM       0x55
#define SKIP_ROM        0xcc
#define ALARM_SEARCH    0xec

/* function commands */
#define CONVERT_T               0x44
#define WRITE_SCRATCHPAD        0x4e
#define READ_SCRATCHPAD         0xbe
#define COPY_SCRATCHPAD         0x48
#define RECALL_E2               0xb8
#define READ_POWER_SUPPLY       0xb4


#define CRC_MASK                0xff00000000000000L
#define SERIAL_MASK             0x00ffffffffffff00L
#define FAMILY_MASK             0x00000000000000ffL


/* registers */
#define TEMPLSB 0
#define TEMPMSB 1
#define THIGH   2
#define TLOW    3
#define CONFIG  4
// reserved     5
// reserved     6
// reserved     7
#define CRC     8

#define MAX_RETRIES 2


/* mask values to filter out higher resolution bits */
const uint8_t fmask[4] = {0x08, 0x0c, 0x0e, 0x0f};


/* I/O access */
int mem_fd;
void *gpio_map;
volatile unsigned *gpio;


/*
** nominally private routines
*/

void _delay_us(uint delay)
{
    int elapsed;
    struct timeval t1, t2;

    elapsed = 0;
    gettimeofday(&t1, NULL);
    while (elapsed < delay)
    {
        gettimeofday(&t2, NULL);
        elapsed = t2.tv_usec - t1.tv_usec;
        if (elapsed < 0)
            elapsed += 1000000;
    }
}


int _setup_io(void)
{
    /* open /dev/gpiomem */
    if ((mem_fd = open("/dev/gpiomem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/gpiomem \n");
        exit(-1);
    }

    /* mmap GPIO */
    gpio_map = mmap(
        NULL,             //Any adddress in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        GPIO_BASE         //Offset to GPIO peripheral
    );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (gpio_map == MAP_FAILED)
        return -1;

    // Always use volatile pointer!
    gpio = (volatile unsigned *)gpio_map;

    return 0;
}


uint8_t _reset(ds18b20_t *p)
{
/* return TRUE if DS18B20 device detected, FALSE if not detected */
    uint8_t   pin;
    uint8_t   dq;

    pin = p->pin;
    dq = 1;

    // pull bus low for Trstl (480us)
    GPIO_CLR = 1 << pin;
    OUT_GPIO(pin);
    _delay_us(Trstl);

    // then release and wait a bit before sampling
    INP_GPIO(pin);
    _delay_us(Trstwait);
    dq = GET_GPIO(pin);

    /* need to wait the rest of the Trsth time */
    _delay_us(Trsth - Trstwait);

    /* DQ=LOW  <-> present (return TRUE) */
    /* DQ=HIGH <-> not present (return FALSE) */
    return (dq ? FALSE : TRUE);
}


uint8_t _write_byte(ds18b20_t *p, uint8_t data)
{
/* bytes are written LSB first */
    uint8_t   pin;

    uint8_t n;
    uint8_t ch;
    uint8_t bit;

    pin  = p->pin;

    ch = data;
    for (n = 0; n < 8; n++)
    {
        bit = ch % 2;

        if (bit == 0)
        {
            /*
            ** write 0
            ** pull bus low for Tlow0 (60us) then release
            */
            GPIO_CLR = 1 << pin;
            OUT_GPIO(pin);
            _delay_us(Tlow0);
            INP_GPIO(pin);
        }
        else
        {
            /* write 1 */
            /* pull bus low for Tlow1, then release and wait rest of slot */
            GPIO_CLR = 1 << pin;
            OUT_GPIO(pin);
            _delay_us(Tlow1);
            INP_GPIO(pin);
            _delay_us(Tslot - Tlow1);
        }

        /* must wait Trec time after each bit is transmitted */
        _delay_us(Trec);

        ch = ch >> 1;
    }

    return 0;
}


uint8_t _read_bit(ds18b20_t *p)
{

    int     n;
    uint8_t pin;
    uint8_t ch;

    pin  = p->pin;

    ch = 0;

    /*
    ** read slot; pull bus low for Tread (1us)
    ** then release and wait a bit before sampling
    ** should sample close to but before expiration of Trdv
    */
    GPIO_CLR = 1 << pin;
    OUT_GPIO(pin);
    _delay_us(Tread);

    INP_GPIO(pin);
    _delay_us(Trdv - Tread - 2);
    ch = GET_GPIO(pin);

    /* then wait the rest of slot + recovery time */
    _delay_us(Tslot - Trdv + 2 + Trec);

    return ch;
}



uint8_t _read_byte(ds18b20_t *p)
{

    int     n;
    uint8_t pin;
    uint8_t ch;
    uint8_t data;

    pin  = p->pin;

    ch = 0;
    data = 0;

    for (n = 0; n < 8; n++)
    {
        ch = _read_bit(p) << n;
        data |= ch;
    }

    return data;
}



/*
** nominally public routines
*/

uint8_t ds18b20_init(ds18b20_t *p)
{
/* struct *p must have port and pin values set before this function is called */
    uint8_t pin;

    _setup_io();
    p->configvalid = FALSE;
    p->present = FALSE;

    /* initialise resolution to 16 bit */
    p->scratchpad[CONFIG] = DS_RES_16;

    pin = p->pin;

    if (pin >= MAX_GPIO)
        return -1;

    INP_GPIO(pin);

    p->present = _reset(p);
    return p->present;
}


int16_t ds18b20_read_temperature(ds18b20_t *p)
{
    int n;
    int attempt;
    uint8_t crc;
    uint8_t resolution;
    uint8_t res_index;
    uint8_t fraction;
    int8_t  tempint;
    int16_t temp16;

    temp16 = ENAVAIL;
    attempt = 0;
    while (attempt < MAX_RETRIES && (temp16 < TEMP_MIN || temp16 > TEMP_MAX))
    {
        /* reset */
        p->present = _reset(p);
        if (!p->present)
        {
            temp16 = ENOTPRESENT;
            attempt++;
            continue;
        }

        /* then send SKIP_ROM, CONVERT_T */
        _write_byte(p, SKIP_ROM);
        _write_byte(p, CONVERT_T);

        /*  wait time for temperature conversion is based on resolution */
        res_index = p->scratchpad[CONFIG] / 32;
        if (res_index > 3)
            res_index = 3;
        _delay_us(Tconv[res_index]);

        /*  if temperature conversion is still not complete, return an error */
        if (_read_bit(p) == 0)
        {
            temp16 = ENAVAIL;
            attempt++;
            continue;
        }

        /* now send reset, SKIP_ROM, READ_SCRATCHPAD */
        _reset(p);
        _write_byte(p, SKIP_ROM);
        _write_byte(p, READ_SCRATCHPAD);

        /* and read the result */
        for (n = 0; n < SCRATCHPAD_SIZE; n++)
            p->scratchpad[n] = _read_byte(p);

        /* check if the CRC is correct */
        crc = crc1w(SCRATCHPAD_SIZE, p->scratchpad);
        if (crc != 0)
        {
            temp16 = EBADCRC;
            attempt++;
            continue;
        }

        /*
        ** if the resolution is less than 1/16, there may be high res bits
        ** left over from previous readings so we should filter them out
        */
        resolution = p->scratchpad[CONFIG];
        res_index = resolution / 32;
        if (res_index > 3)
            res_index = 3;
        fraction = p->scratchpad[TEMPLSB] & fmask[res_index];

        /*
        ** the integer portion of temperature fits into one byte but is
        ** stored as 4 bits in LSB and 4 bits in MSB.
        ** remove the fraction bits by shifting right 4 bits and then
        ** insert the 4 bits from MSB into the tempint variable
        */
        tempint = p->scratchpad[TEMPLSB] >> 4;
        tempint |= (p->scratchpad[TEMPMSB] << 4);

        p->tempint = tempint;
        p->tempfrac = fraction;
        p->temphigh = p->scratchpad[THIGH];
        p->templow = p->scratchpad[TLOW];
        p->resolution = resolution;
        p->configvalid = TRUE;

        /* return the temperature expressed as multiples of 1/16th degree C */
        temp16 = tempint * 16 + fraction;
        attempt++;
    }

    return temp16;
}

