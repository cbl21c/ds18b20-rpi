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
    tempint = temp16 / 16;
    tempfrac = temp16 & 0x0f;
    tempfloat = (double) tempint + (double) tempfrac / 16.0;
    printf("%.4f\n", tempfloat);
    // printf("%d:%d\n", tempint, tempfrac);

    return 0;
}

