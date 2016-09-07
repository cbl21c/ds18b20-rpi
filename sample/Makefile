CC     = /usr/bin/gcc
CFLAGS = -O2 -Wall
LIBS   = -lds18b20
TARGET = readtemp

all:	$(TARGET)

$(TARGET):	$(TARGET).o
		$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o

