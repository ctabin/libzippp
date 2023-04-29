cmake_minimum_required(VERSION 3.16.0)

project(libzippp
        VERSION 6.0.0
        DESCRIPTION "C++ wrapper for libzip"
        HOMEPAGE_URL "http://github.com/ctabin/libzippp"
        LANGUAGES CXX
)

# Use C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Do not install or build tests by default when this file is included
# as a sub-project (via add_subdirectory)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(is_root_project ON)
else()
  set(is_root_project OFF)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -ansi -pedantic -std=c++11")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

option(LIBZIPPP_INSTALL "Install library" ${is_root_project})
option(LIBZIPPP_INSTALL_HEADERS "Install the headers" ${is_root_project})
option(LIBZIPPP_BUILD_TESTS "Build unit tests" ${is_root_project})
option(LIBZIPPP_ENABLE_ENCRYPTION "Build with encryption enabled" OFF)
option(LIBZIPPP_CMAKE_CONFIG_MODE "Build with libzip installed cmake config files" OFF)
option(LIBZIPPP_GNUINSTALLDIRS "Install into directories taken from GNUInstallDirs" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(LIBZIPPP_CMAKE_CONFIG_MODE)
  set(LIBZIP_PKGNAME "libzip")
  set(fp_mode "CONFIG")
else()
  set(LIBZIP_PKGNAME "LIBZIP")
  set(fp_mode "MODULE")
endif()

find_package(${LIBZIP_PKGNAME} ${fp_mode} REQUIRED)

if(LIBZIPPP_GNUINSTALLDIRS)
  include(GNUInstallDirs)
  set(install_include_location ${CMAKE_INSTALL_INCLUDEDIR}/libzippp)
else()
  set(install_include_location include/libzippp)
endif()

add_library(libzippp "src/libzippp.cpp")
add_library(libzippp::libzippp ALIAS libzippp) # Convenience alias
target_include_directories(libzippp
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${install_include_location}>

)
set_target_properties(libzippp PROPERTIES PREFIX "") # Avoid duplicate "lib" prefix
target_link_libraries(libzippp PRIVATE libzip::zip)

if(LIBZIPPP_ENABLE_ENCRYPTION)
  target_compile_definitions(libzippp PUBLIC LIBZIPPP_WITH_ENCRYPTION)
endif()

if (BUILD_SHARED_LIBS)
  target_compile_definitions(libzippp PRIVATE LIBZIPPP_EXPORTS)
else()
  set_target_properties(libzippp PROPERTIES OUTPUT_NAME "libzippp_static")
endif()

export(TARGETS libzippp NAMESPACE libzippp:: FILE ${PROJECT_BINARY_DIR}/libzippp-target.cmake)

if(LIBZIPPP_BUILD_TESTS)
  enable_testing()
  add_executable(libzippp_test "tests/tests.cpp")
  target_link_libraries(libzippp_test PRIVATE libzippp)
  add_test(NAME libzippp_tests COMMAND libzippp_test)

  if (BUILD_SHARED_LIBS)
    set_target_properties(libzippp_test PROPERTIES OUTPUT_NAME "libzippp_shared_test")
  else()
    set_target_properties(libzippp_test PROPERTIES OUTPUT_NAME "libzippp_static_test")
  endif()
endif()

if(LIBZIPPP_INSTALL)
  install(
    TARGETS libzippp
    EXPORT libzipppTargets
  )

  if(LIBZIPPP_INSTALL_HEADERS)
    install(
      FILES src/libzippp.h
      DESTINATION ${install_include_location}
    )
  endif()

  include(CMakePackageConfigHelpers)

  set(PROJECT_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/${PROJECT_NAME}Config.cmake")
  set(PROJECT_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/${PROJECT_NAME}ConfigVersion.cmake")
  if(LIBZIPPP_GNUINSTALLDIRS)
    set(configInstallDestination ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
  else()
    if(WIN32)
      set(configInstallDestination cmake/${PROJECT_NAME})
    else()
      set(configInstallDestination share/${PROJECT_NAME})
    endif()
  endif()

  configure_package_config_file(
    "Config.cmake.in"
    ${PROJECT_CONFIG_FILE}
    INSTALL_DESTINATION ${configInstallDestination}
  )
  write_basic_package_version_file(
    ${PROJECT_VERSION_FILE}
    COMPATIBILITY SameMajorVersion
  )

  install(
    FILES ${PROJECT_CONFIG_FILE} ${PROJECT_VERSION_FILE}
    $<$<NOT:$<BOOL:"${LIBZIPPP_CMAKE_CONFIG_MODE}">>:cmake/FindLIBZIP.cmake>
    DESTINATION ${configInstallDestination}
  )

  install(
    EXPORT libzipppTargets
    NAMESPACE "${PROJECT_NAME}::"
    DESTINATION ${configInstallDestination}
  )

  if(BUILD_SHARED_LIBS AND LIBZIPPP_GNUINSTALLDIRS AND LIBZIPPP_INSTALL_HEADERS)
    string(
      REGEX REPLACE "^${CMAKE_INSTALL_PREFIX}" "\${prefix}"
      pc_includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}"
    )
    string(
      REGEX REPLACE "^${CMAKE_INSTALL_PREFIX}" "\${exec_prefix}"
      pc_libdir "${CMAKE_INSTALL_FULL_LIBDIR}"
    )
    configure_file(libzippp.pc.in generated/libzippp.pc @ONLY)
    install(
      FILES ${CMAKE_CURRENT_BINARYDIR}/generated/libzipp.pc
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
  endif()
endif()
