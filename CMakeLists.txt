
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(libzippp)

SET(PACKAGE "libzippp")
SET(VERSION "0.1")

SET(LIBZIP_PATH "lib/libzip-0.11.1" CACHE STRING "Path to libzip")

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
set(CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIR})
IF(ZLIB_VERSION_STRING VERSION_LESS "1.1.2")
  MESSAGE(FATAL_ERROR "-- ZLIB version too old, please install at least v1.1.2")
ENDIF(ZLIB_VERSION_STRING VERSION_LESS "1.1.2")

INCLUDE_DIRECTORIES(${LIBZIP_PATH}/lib)
INCLUDE_DIRECTORIES("src")

ADD_LIBRARY(libzippp_shared SHARED "src/libzippp.cpp")
SET_TARGET_PROPERTIES(libzippp_shared PROPERTIES OUTPUT_NAME libzippp)
TARGET_LINK_LIBRARIES(libzippp_shared 
						debug ../${LIBZIP_PATH}/build/lib/Debug/zip
						optimized ../${LIBZIP_PATH}/build/lib/Release/zip
						${ZLIB_LIBRARY})


ADD_LIBRARY(libzippp_static STATIC "src/libzippp.cpp")
SET_TARGET_PROPERTIES(libzippp_static PROPERTIES OUTPUT_NAME libzippp)
TARGET_LINK_LIBRARIES(libzippp_static 
						debug ../${LIBZIP_PATH}/build/lib/Debug/zip
						optimized ../${LIBZIP_PATH}/build/lib/Release/zip
						${ZLIB_LIBRARY})

ADD_EXECUTABLE(libzippp_test "tests/tests.cpp")
TARGET_LINK_LIBRARIES(libzippp_test libzippp_shared)
