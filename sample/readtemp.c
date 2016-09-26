#include <stdio.h>
#include <sensor/ds18b20.h>

int main(void)
{
    int temp16;
    int tempint;
    int tempfrac;
    int found;
    double tempfloat;
    ds18b20_t dev;

    dev.pin = 4;
    found = ds18b20_init(&dev);
    if (!found)
    {
        fprintf(stderr, "DS18B20 not present\n");
        return -1;
    }

    temp16 = ds18b20_read_temperature(&dev);

    if (temp16 < TEMP_MIN || temp16 > TEMP_MAX)
    {
        switch(temp16)
        {
            case ENOTPRESENT:
                fprintf(stderr, "DS18B20 not present\n");
                break;

            case ENAVAIL:
                fprintf(stderr, "Temperature reading not available\n");
                break;

            case EBADCRC:
                fprintf(stderr, "CRC error\n");
                break;

            default:
                fprintf(stderr, "Unknown error: %d\n", temp16);
                break;
        }

        return -1;
    }

    /* temperature is within range so continue processing */
    tempint = temp16 / 16;
    tempfrac = temp16 & 0x0f;
    tempfloat = (double) tempint + (double) tempfrac / 16.0;
    printf("%.4f\n", tempfloat);
    // printf("%d:%d\n", tempint, tempfrac);

    return 0;
}

