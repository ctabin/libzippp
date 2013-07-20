CC=g++
CFLAGS=-W -Wall -Wextra -ansi -pedantic
OBJ=obj
LIB=lib

all: libzippp-static libzippp-shared

libzippp-compile:
	rm -rf $(OBJ)
	mkdir $(OBJ)
	$(CC) -fPIC -c -I./$(LIB)/libzip-0.11.1/lib -o $(OBJ)/libzippp.o $(CFLAGS) src/libzippp.cpp

libzippp-static: libzippp-compile
	ar rvs libzippp.a $(OBJ)/libzippp.o

libzippp-shared: libzippp-compile
	$(CC) -shared -o libzippp.so $(OBJ)/libzippp.o

libzippp-tests: libzippp-static
	$(CC) -o test -I./$(LIB)/libzip-0.11.1/lib -I./src $(CFLAGS) tests/tests.cpp libzippp.a $(LIB)/libzip-0.11.1/lib/.libs/libzip.a -lz

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

libzip-compile: libzip-unzip
	cd $(LIB)/libzip-0.11.1 && ./configure && make

libzip: libzip-compile
