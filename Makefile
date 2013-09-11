CC=g++
CFLAGS=-W -Wall -Wextra -ansi -pedantic
OBJ=obj
LIB=lib
LIBZIP=$(LIB)/libzip-0.11.1

all: libzippp-static libzippp-shared

libzippp-compile:
	rm -rf $(OBJ)
	mkdir $(OBJ)
	$(CC) -fPIC -c -I$(LIBZIP)/lib -o $(OBJ)/libzippp.o $(CFLAGS) src/libzippp.cpp

libzippp-static: libzippp-compile
	ar rvs libzippp.a $(OBJ)/libzippp.o

libzippp-shared: libzippp-compile
	$(CC) -shared -o libzippp.so $(OBJ)/libzippp.o

libzippp-tests: libzippp-static
	$(CC) -o test -I$(LIBZIP)/lib -Isrc $(CFLAGS) tests/tests.cpp libzippp.a $(LIBZIP)/lib/.libs/libzip.a -lz

clean-tests:
	@rm -rf *.zip

tests: libzippp-tests clean-tests
	./test

clean:
	@rm -rf libzippp.a libzippp.so
	@rm -rf $(OBJ)
	@rm -rf test

mrproper: clean
	@rm -rf $(LIB)

libzip-init:
	mkdir -p $(LIB)

libzip-download: libzip-init
	wget -c -O "$(LIB)/libzip-0.11.1.tar.gz" "http://www.nih.at/libzip/libzip-0.11.1.tar.gz"

libzip-unzip: libzip-download
	cd $(LIB) && tar -xf libzip-0.11.1.tar.gz

libzip-configure: libzip-unzip
	cd $(LIBZIP) && ./configure

libzip-patch: libzip-configure
	cd $(LIBZIP) && patch -p1 < ../../patch/libzip-windows.patch

libzip-compile: libzip-patch
	cd $(LIBZIP) && make

libzip: libzip-compile
