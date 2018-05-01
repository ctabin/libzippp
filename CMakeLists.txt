
CMAKE_MINIMUM_REQUIRED(VERSION 3.9.5)

PROJECT(libzippp)

SET(PACKAGE "libzippp")
SET(VERSION "0.2")

if(NOT WIN32)
  MESSAGE(FATAL_ERROR "This CMakeLists is only designed for Windows. Under UNIX, use make instead")
endif()

set(LIBZIP_INCLUDE_DIR "${LIBZIP_HOME}/lib;${LIBZIP_HOME}/build" CACHE PATH "Path to libzip include folder.")
set(LIBZIP_LIBRARY_RELEASE "../${LIBZIP_HOME}/build/lib/Release/zip" CACHE FILEPATH "Path to libzip release library.")
set(LIBZIP_LIBRARY_DEBUG "../${LIBZIP_HOME}/build/lib/Debug/zip" CACHE FILEPATH "Path to libzip debug library.")

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
set(CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIR})
IF(ZLIB_VERSION_STRING VERSION_LESS "1.1.2")
  MESSAGE(FATAL_ERROR "-- ZLIB version too old, please install at least v1.1.2")
ENDIF(ZLIB_VERSION_STRING VERSION_LESS "1.1.2")

INCLUDE_DIRECTORIES(${LIBZIP_INCLUDE_DIR})
set(CMAKE_REQUIRED_INCLUDES ${LIBZIP_INCLUDE_DIR})

add_definitions(-DLIBZIPPP_EXPORTS)
ADD_LIBRARY(libzippp_shared SHARED "src/libzippp.cpp")
SET_TARGET_PROPERTIES(libzippp_shared PROPERTIES OUTPUT_NAME libzippp)
TARGET_LINK_LIBRARIES(libzippp_shared 
			debug ${LIBZIP_LIBRARY_DEBUG}
			optimized ${LIBZIP_LIBRARY_RELEASE}
			${ZLIB_LIBRARY})

ADD_LIBRARY(libzippp_static STATIC "src/libzippp.cpp")

ADD_EXECUTABLE(libzippp_shared_test "tests/tests.cpp")
TARGET_LINK_LIBRARIES(libzippp_shared_test libzippp_shared)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
ADD_EXECUTABLE(libzippp_static_test "tests/tests.cpp")
TARGET_LINK_LIBRARIES(libzippp_static_test 
                        libzippp_static
                        debug ${LIBZIP_LIBRARY_DEBUG}
                        optimized ${LIBZIP_LIBRARY_RELEASE}
                        debug ${ZLIB_LIBRARY_DEBUG}
                        optimized ${ZLIB_LIBRARY_RELEASE})
