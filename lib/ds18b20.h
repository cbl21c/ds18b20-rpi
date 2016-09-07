#ifndef __ds18b20_h__
#define __ds18b20_h__


#include <sys/types.h>

/* timers (usec) for reading and writing to DS18B20 */
#define Tconv		750000	// maximum
#define Tslot		60	// minimum
#define Trec		1	// minimum
#define Tlow0		60	// minimum
#define Tlow1		1	// minimum
#define Trdv		15	// maximum
#define Trsth		480	// minimum
#define Trstl		480	// minimum
#define Tpdh		15	// minimum
#define Tpdl		60	// minimum
#define Tread		1	// minimum
#define Trstwait	65	// typical
#define Twr		10000	// maximum

/* mask and size values */
#define DS_RES_2	0x1f
#define DS_RES_4	0x3f
#define DS_RES_8	0x5f
#define DS_RES_16	0x7f
#define SCRATCHPAD_SIZE	9
#define MAX_GPIO	28

/* error codes */
#define EBADR		53	/* invalid port or pin */
#define ENOTPRESENT	10404	/* DS18B20 device is not connected */
				/* this value needs to be outside the range 16*[-55:125] */


typedef u_int8_t uint8_t;

typedef struct ds18b20
{
    int8_t    present;

    uint8_t   pin;

    int8_t    tempint;
    int8_t    tempfrac;
    int8_t    temphigh;
    int8_t    templow;
    int8_t    configvalid;
    uint8_t   resolution;

    uint8_t   scratchpad[SCRATCHPAD_SIZE];
} ds18b20_t;


extern uint8_t ds18b20_init(ds18b20_t*);
extern int16_t ds18b20_read_temperature(ds18b20_t*);


#endif

