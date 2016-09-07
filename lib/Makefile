AR     = /usr/bin/ar
CC     = /usr/bin/gcc
CFLAGS = -O2
OBJS   = ds18b20.o
TARGET = libds18b20.a


lib:	ds18b20.o
	$(AR) cr libds18b20.a ds18b20.o

.c.o:
	$(CC) $(CFLAGS) -c $<

install:	libds18b20.a
		sudo cp ds18b20.h /usr/local/include/sensor
		sudo cp libds18b20.a /usr/local/lib

clean:
	rm -f *.hex *.o *.a

