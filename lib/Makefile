MCU        = attiny84
AVRDUDEMCU = t84
AR         = /usr/avr/bin/avr-ar
CC         = /usr/avr/bin/avr-gcc
CFLAGS     = -Os -Wall -mcall-prologues -mmcu=$(MCU)
OBJS       = $(TARGET).o 
TARGET     = libds18b20.a


lib:	ds18b20.o
	$(AR) cr libds18b20.a ds18b20.o

.c.o:
	$(CC) $(CFLAGS) -c $<

install:	libds18b20.a
		sudo cp ds18b20.h /usr/avr/include/sensor
		sudo cp libds18b20.a /usr/avr/lib/gcc/avr/4.9.2

clean:
	rm -f *.hex *.o *.a

