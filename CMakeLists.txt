
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(libzippp)

SET(PACKAGE "libzippp")
SET(VERSION "0.2")

SET(ZLIB_PATH "lib/zlib-1.2.8" CACHE STRING "Path to zlib")
SET(LIBZIP_PATH "lib/libzip-1.1.2" CACHE STRING "Path to libzip")

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
set(CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIR})
IF(ZLIB_VERSION_STRING VERSION_LESS "1.1.2")
  MESSAGE(FATAL_ERROR "-- ZLIB version too old, please install at least v1.1.2")
ENDIF(ZLIB_VERSION_STRING VERSION_LESS "1.1.2")

INCLUDE_DIRECTORIES(${LIBZIP_PATH}/lib)
INCLUDE_DIRECTORIES(${LIBZIP_PATH}/build)
INCLUDE_DIRECTORIES("src")

add_definitions(-DLIBZIPPP_EXPORTS)
ADD_LIBRARY(libzippp_shared SHARED "src/libzippp.cpp")
SET_TARGET_PROPERTIES(libzippp_shared PROPERTIES OUTPUT_NAME libzippp)
TARGET_LINK_LIBRARIES(libzippp_shared 
			debug ../${LIBZIP_PATH}/build/lib/Debug/zip
			optimized ../${LIBZIP_PATH}/build/lib/Release/zip
			${ZLIB_LIBRARY})

add_definitions(-DLIBZIPPP_EXPORTS)
ADD_LIBRARY(libzippp_static STATIC "src/libzippp.cpp")

ADD_EXECUTABLE(libzippp_shared_test "tests/tests.cpp")
TARGET_LINK_LIBRARIES(libzippp_shared_test libzippp_shared)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd" FORCE)
ADD_EXECUTABLE(libzippp_static_test "tests/tests.cpp")
TARGET_LINK_LIBRARIES(libzippp_static_test 
                        libzippp_static
                        debug ../${LIBZIP_PATH}/build/lib/Debug/zip
			optimized ../${LIBZIP_PATH}/build/lib/Release/zip
			debug ../${ZLIB_PATH}/build/Debug/zlibstaticd
			optimized ../${ZLIB_PATH}/build/Release/zlibstatic)
