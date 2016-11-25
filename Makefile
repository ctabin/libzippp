CC=g++
CFLAGS=-W -Wall -Wextra -ansi -pedantic
OBJ=obj
LIB=lib
ZLIB_VERSION=1.2.8
ZLIB=$(LIB)/zlib-$(ZLIB_VERSION)
LIBZIP_VERSION=1.1.3
LIBZIP=$(LIB)/libzip-$(LIBZIP_VERSION)

# for optimal compilation speed, should be <nb_proc>+1
NBPROC=5

all: libzippp-static libzippp-shared

libzippp-compile:
	rm -rf $(OBJ)
	mkdir $(OBJ)
	$(CC) -g -fPIC -c -I$(LIBZIP)/lib -o $(OBJ)/libzippp.o $(CFLAGS) src/libzippp.cpp

libzippp-static: libzippp-compile
	ar rvs libzippp.a $(OBJ)/libzippp.o

libzippp-shared: libzippp-compile
	$(CC) -shared -o libzippp.so $(OBJ)/libzippp.o

libzippp-tests: libzippp-static libzippp-shared
	if [ -d $(ZLIB) ]; then \
		$(CC) -o test_static -I$(ZLIB) -I$(LIBZIP)/lib -Isrc $(CFLAGS) tests/tests.cpp libzippp.a $(LIBZIP)/lib/.libs/libzip.a $(ZLIB)/libz.a; \
		$(CC) -o test_shared -I$(ZLIB) -I$(LIBZIP)/lib -Isrc $(CFLAGS) tests/tests.cpp -L. -L$(LIBZIP)/lib/.libs -L$(ZLIB) -lzippp -lzip -lz -Wl,-rpath=.; \
	else \
		$(CC) -o test_static -I$(LIBZIP)/lib -Isrc $(CFLAGS) tests/tests.cpp libzippp.a $(LIBZIP)/lib/.libs/libzip.a -lz; \
		$(CC) -o test_shared -I$(LIBZIP)/lib -Isrc $(CFLAGS) tests/tests.cpp -L. -L$(LIBZIP)/lib/.libs -lzippp -lzip -lz -Wl,-rpath=.; \
	fi;

clean-tests:
	@rm -rf *.zip

tests: libzippp-tests clean-tests
	LD_LIBRARY_PATH="$(LIBZIP)/lib/.libs" valgrind --suppressions=ld.supp ./test_shared
	valgrind --suppressions=ld.supp ./test_static

clean:
	@rm -rf libzippp.a libzippp.so
	@rm -rf $(OBJ)
	@rm -rf test_shared test_static

mrproper: clean
	@rm -rf $(LIB)/libzip-1.1
	@rm -rf $(LIB)/libzip-1.1.tar.gz
	
# ZLIB targets
	
zlib-init:
	mkdir -p $(LIB)
	
zlib-download: zlib-init
	wget -c -O "$(ZLIB).tar.gz" "http://zlib.net/zlib-$(ZLIB_VERSION).tar.gz"

zlib-unzip: zlib-download
	cd $(LIB) && tar -xf zlib-$(ZLIB_VERSION).tar.gz

zlib-configure: zlib-unzip
	cd $(ZLIB) && ./configure

zlib-compile: zlib-configure
	cd $(ZLIB) && make -j$(NBPROC)

zlib: zlib-compile

# LIZIP targets

libzip-init:
	mkdir -p $(LIB)

libzip-download: libzip-init
	wget -c -O "$(LIBZIP).tar.gz" "http://www.nih.at/libzip/libzip-$(LIBZIP_VERSION).tar.gz"

libzip-unzip: libzip-download
	cd $(LIB) && tar -xf libzip-$(LIBZIP_VERSION).tar.gz

libzip-patch: libzip-unzip
	if [ -f $(LIB)/libzip-$(LIBZIP_VERSION)-linux.patch ]; then \
		cd $(LIB)/libzip-$(LIBZIP_VERSION) && patch -p1 < ../libzip-$(LIBZIP_VERSION)-linux.patch; \
	fi;

libzip-configure: libzip-patch
	if [ -d "$(ZLIB)" ]; then \
		cd $(LIBZIP) && ./configure --with-zlib=$(abspath $(LIB)/zlib-$(ZLIB_VERSION));  \
	else \
		cd $(LIBZIP) && ./configure; \
	fi;

libzip-compile: libzip-configure
	cd $(LIBZIP) && make -j$(NBPROC)

libzip: libzip-compile

# LIBRARIES TARGET

libraries: zlib libzip
