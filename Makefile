TARFILE = ds18b20_avr.tar

all:
	cd lib && make && make install
	cd sample && make

clean:
	rm $(TARFILE)
	cd lib && make clean
	cd sample && make clean

tar:
	tar -cf $(TARFILE) lib sample Makefile
