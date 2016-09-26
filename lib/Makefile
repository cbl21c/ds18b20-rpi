CC     = /usr/bin/gcc
CFLAGS = -O2 -fPIC
SRC    = ds18b20.c ds18b20.h gpio.h crc1w.c crc1w.h
OBJS   = ds18b20.o crc1w.o
TARGET = libds18b20.so


$(TARGET):	$(OBJS)
		$(CC) -shared $(OBJS) -o $(TARGET)

.c.o:	$(SRC)
	$(CC) $(CFLAGS) -c $<

install:
		sudo cp ds18b20.h /usr/local/include/sensor
		sudo cp libds18b20.so /usr/local/lib
		sudo ldconfig

clean:
	rm -f *.o *.a *.so

