[![Build Status](https://travis-ci.org/ctabin/libzippp.svg?branch=master)](https://travis-ci.org/ctabin/libzippp)

# LIBZIPPP

libzippp is a simple basic C++ wrapper around the libzip library.
It is meant to be a portable and easy-to-use library for ZIP handling.

Compilation has been tested with:
- GCC 9.2.1 (GNU/Linux Debian) 
- MS Visual Studio 2012 (Windows 7)

Underlying libraries:
- [ZLib](http://zlib.net) 1.2.11
- [libzip](http://www.nih.at/libzip) 1.6.1
- Optional: [BZip2](https://www.sourceware.org/bzip2/)

## Integration

libzippp has been ported  to [vcpkg](https://github.com/microsoft/vcpkg) and thus can be
very easily integrated by running:
```
./vcpkg install libzippp
```

## Compilation

### Install Prerequisites

- Linux
  - Install the development packages for zlib and libzip (e.g. `zlib1g-dev`, `libzip-dev`, `liblzma-dev`, `libbz2-dev`)
  - OR Install from source
  - OR Use the utility in the Makefile by executing `make libraries`
- Windows:
  - Use precompile libraries from *libzip-\<version\>-windows-ready_to_compile.zip*
  - Install from source via CMake (similar to workflow below)

### Compile libzippp

TLDR: Use the standard CMake workflow: `mkdir build && cd build && cmake <-D...> .. && make install`

- Make sure you have a compiler (MSVC, g++, ...) and CMake installed
- Switch to the source folder
- Create a build folder and switch to it, e.g.: `mkdir build && cd build`
- Configure the build with cmake:
  - Commandline: `cmake .. -DCMAKE_BUILD_TYPE=Release`
  - With the CMake GUI:
    - Set source and build folder accordingly
	- Click `Add Cache Entry` to add `CMAKE_BUILD_TYPE` if not building with MSVC
	- Click `Configure` & `Generate`
  - If CMake can't find zlib and/or libzip you need to set `CMAKE_PREFIX_PATH` to the directories where you installed those into
  (either via `-DCMAKE_PREFIX_PATH=<...>` or via the GUI)
    - Example: `-DCMAKE_PREFIX_PATH=/home/user/libzip-1.6.1:/home/user/zlib-1.2.11`
- Compile as usual
  - Linux: `make && make install`
  - Windows: Open generated project in MSVC. Build the `INSTALL` target to install.

### CMake variables of interest

Set via commandline as `cmake -DNAME=VALUE <other opts>` or via CMake GUI or CCMake `Add Cache Entry`.

- `LIBZIPPP_INSTALL`: Enable/Disable installation of libzippp. Default is OFF when using via `add_subdirectory`, else ON
- `LIBZIPPP_INSTALL_HEADERS`: Enable/Disable installation of libzippp headers. Default is OFF when using via `add_subdirectory`, else ON
- `LIBZIPPP_BUILD_TESTS`: Enable/Disable building libzippp tests. Default is OFF when using via `add_subdirectory`, else ON
- `CMAKE_INSTALL_PREFIX`: Where to install the project to
- `CMAKE_BUILD_TYPE`: Set to Release or Debug to build with or without optimizations
- `BUILD_SHARED_LIBS`: Set to ON or OFF to build shared or static libs, uses platform default if not set
- `CMAKE_PREFIX_PATH`: Colon-separated list of prefix paths (paths containing `lib` and `include` folders) for installed libs to be used by this

### Using libzippp

Once installed libzipp can be used from any CMake project with ease:   
Given that it was installed (via `CMAKE_INSTALL_PREFIX`) into a standard location or its install prefix is passed into your projects
`CMAKE_PREFIX_PATH` you can simply call `find_package(libzippp 3.0 REQUIRED)` and link against `libzipp::libzipp`.

When not using CMake to consume libzipp you have to pass its include directory to your compiler and link against `libzippp.{a,so}`.
Do not forget to also link against libzip libraries e.g. in *lib/libzip-1.6.1/lib/.libs/*).
An example of compilation with g++:
  
```shell
g++ -I./lib/libzip-1.6.1/lib -I./src \
    main.cpp libzippp.a \
    lib/libzip-1.6.1/lib/.libs/libzip.a \
    lib/zlib-1.2.11/libz.a
```

#### Encryption

Since version 1.5, libzip uses an underlying cryptographic library (OpenSSL, GNUTLS or CommonCrypto) that
is necessary for static compilation. By default, libzippp will use `-lssl -lcrypto` (OpenSSL) as default flags
to compile the tests. This can be changed by using `make CRYPTO_FLAGS="-lsome_lib" LIBZIP_CMAKE="" tests`.

Since libzip `cmake`'s file detects automatically the cryptographic library to use, by default all the allowed
libraries but OpenSSL are explicitely disabled in the `LIBZIP_CMAKE` variable in the Makefile.

See [here](https://github.com/nih-at/libzip/blob/master/INSTALL.md) for more information.

### WINDOWS - Alternative way

The easiest way is to download zlib, libzip and libzipp sources and use CMake GUI to build each library in order:

- Open CMake GUI
- Point `Source` to the libraries source folder, `Build` to a new folder `build` inside it
- Run `Generate`
- Open the generated solution in MSVC and build & install it
- Repeat for the next library

But there is also a prepared batch file to help automate this.
It may need some adjusting though.

#### From Stage 1 - Use prepared environment


0. Make sure you have cmake 3.10 (*cmake.exe* must be in the PATH) and MS Visual Studio.

1. Download the *libzip-\<version\>-windows-ready_to_compile.zip* file from the release 
  and extract it somewhere on your system. This will create a prepared structure, so *libzippp* can 
  be compiled along with the needed libraries.

2. Simply execute the *compile.bat* file. This will compile *zlib*, *libzip* and
 finally *libzippp*.

3. You'll have a *dist* folder containing the *release* and *debug* folders 
  where you can now execute the libzippp tests.

#### From Stage 0 - DIY

0. Make sure you have cmake 3.10 (*cmake.exe* must be in the PATH) and MS Visual Studio 2012.
  
1. Download [libzip](http://www.nih.at/libzip/libzip-1.6.1.tar.gz) and [zlib](http://zlib.net/zlib1211.zip) sources and extract them in the 'lib' folder.
  You should end up with the following structure:
  ```
  libzippp/compile.bat
  libzippp/lib/zlib-1.2.11
  libzippp/lib/libzip-1.6.1
  ```
2. Apply the modifications described in libzippp/lib/libzip-1.6.1-windows.patch.

3. Execute the *compile.bat* (simply double-click on it). The compilation should 
  go without error.

4. You'll have a *dist* folder containing the *release* and *debug* folders 
  where you can now execute the libzippp tests.

5. You can either use *libzippp.dll* and *libzippp.lib* to link dynamically the 
  library or simply use *libzippp_static.lib* to link it statically. Unless you 
  also link zlib and libzippp statically, you'll need the dll packaged with 
  your executable.

# Usage 

The API is meant to be very straight forward. Some french explanations
can be found [here](http://www.astorm.ch/blog).

### List and read files in an archive:

```C++
#include "libzippp.h"
using namespace libzippp;

ZipArchive zf("archive.zip");
zf.open(ZipArchive::READ_ONLY);

vector<ZipEntry> entries = zf.getEntries();
vector<ZipEntry>::iterator it;
for(it=entries.begin() ; it!=entries.end(); ++it) {
  ZipEntry entry = *it;
  string name = entry.getName();
  int size = entry.getSize();

  //the length of binaryData will be size
  void* binaryData = entry.readAsBinary();

  //the length of textData will be size
  string textData = entry.readAsText();

  //...
}

zf.close();
```

You can also create an archive directly from a buffer:
```C++
#include "libzippp.h"
using namespace libzippp;

char* buffer = someData;
uint32_t bufferSize = sizeOfBuffer;

ZipArchive* zf = ZipArchive::fromBuffer(buffer, bufferSize);
/* work with zf */
zf->close();
delete zf;
```

### Read a specific entry from an archive:

```C++
#include "libzippp.h"
using namespace libzippp;

ZipArchive zf("archive.zip");
zf.open(ZipArchive::READ_ONLY);

//raw access
char* data = (char*)zf.readEntry("myFile.txt", true);
ZipEntry entry1 = zf.getEntry("myFile.txt");
string str1(data, entry1.getSize());

//text access
ZipEntry entry2 = zf.getEntry("myFile.txt");
string str2 = entry2.readAsText();

zf.close();
```

### Read a large entry from an archive:

```C++
#include "libzippp.h"
using namespace libzippp;

ZipArchive zf("archive.zip");
zf.open(ZipArchive::READ_ONLY);

ZipEntry largeEntry = z1.getEntry("largeentry");
std::ofstream ofUnzippedFile("largeFileContent.data");
largeEntry.readContent(ofUnzippedFile);
ofUnzippedFile.close();

zf.close();
```

### Add data to an archive:

```C++
#include "libzippp.h"
using namespace libzippp;

ZipArchive zf("archive.zip");
zf.open(ZipArchive::WRITE);
zf.addEntry("folder/subdir/");

const char* textData = "Hello,World!";
zf.addData("helloworld.txt", textData, 12);

zf.close();
```

### Remove data from an archive:

```C++
#include "libzippp.h"
using namespace libzippp;

ZipArchive zf("archive.zip");
zf.open(ZipArchive::WRITE);
zf.deleteEntry("myFile.txt");
zf.deleteEntry("myDir/subDir/");
zf.close();
```

## Known issues

### LINUX

You might already have libzip compiled elsewhere on your system. Hence, you
don't need to run 'make libzip'. Instead, just put the libzip location when
you compile libzipp:

```shell
make LIBZIP=path/to/libzip
```

Under Debian, you'll have to install the package `zlib1g-dev` in order to compile
if you don't want to install zlib manually.

### WINDOWS

By default, MS Visual Studio 2012 is installed under the following path:

```
C:\Program Files (x86)\Microsoft Visual Studio 11.0\
```

Be aware that non-virtual-only classes are shared within the DLL of libzippp.
Hence you'll need to use the same compiler for libzippp and the pieces of code
that will use it. To avoid this issue, you'll have to link the library statically.

More information [here](http://www.codeproject.com/Articles/28969/HowTo-Export-C-classes-from-a-DLL).

### Static linkage

Extra explanations can be found [here](http://hostagebrain.blogspot.com/search/label/zlib).

