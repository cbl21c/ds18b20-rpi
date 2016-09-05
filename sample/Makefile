MCU        = attiny84
AVRDUDEMCU = t84
CC         = /usr/avr/bin/avr-gcc
CFLAGS     = -Os -Wall -mcall-prologues -mmcu=$(MCU)
INCLUDE    = -I/usr/avr/include
LIBS       = -lds18b20
OBJ2HEX    = /usr/avr/bin/avr-objcopy
AVRDUDE    = /usr/avr/bin/avrdude
BINARYTEMP = binarytemp
DECTEMP    = dectemp

all:	binarytemp dectemp

binarytemp:	binarytemp.o
		$(CC) $(CFLAGS) -o $(BINARYTEMP) $(BINARYTEMP).o $(LIBS)
		$(OBJ2HEX) -R .eeprom -O ihex $(BINARYTEMP) $(BINARYTEMP).hex
		rm -f $(BINARYTEMP)

dectemp:	dectemp.o display.o
		$(CC) $(CFLAGS) $(INCLUDE) -o $(DECTEMP) $(DECTEMP).o display.o $(LIBS)
		$(OBJ2HEX) -R .eeprom -O ihex $(DECTEMP) $(DECTEMP).hex
		rm -f $(DECTEMP)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

flash:
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -P /dev/spidev0.0 -c linuxspi -b 10000 -U flash:w:$(TARGET).hex

clean:
	rm -f *.hex *.o

tar:
	tar -cf ds18b20_avr.tar *c *h Makefile
