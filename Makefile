BASE=$(PWD)

.PHONY: all clean install test

all:
	mkdir -p build
	cd deps/libjpw; make
	cd src;	make
	cd test; make
test:
	cd test; make
install:
	cp build/libmj.so /usr/local/lib
clean:
	cd deps/libjpw; make clean
	cd src; make clean
	cd test; make clean
	rm -rf build
