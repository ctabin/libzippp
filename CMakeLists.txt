cmake_minimum_required(VERSION 3.8.0)

project(libzippp VERSION 3.0.0)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(is_root_project ON)
else()
    set(is_root_project OFF)
endif()

option(LIBZIPPP_INSTALL "Install library" ${is_root_project})
option(LIBZIPPP_BUILD_TESTS "Build unit tests" ${is_root_project})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(LIBZIP MODULE REQUIRED)

add_library(libzippp "src/libzippp.cpp")
add_library(libzippp::libzippp ALIAS libzippp) # Convenience alias
target_include_directories(libzippp 
  PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/libzippp>
)
set_target_properties(libzippp PROPERTIES PREFIX "") # Avoid duplicate "lib" prefix
target_link_libraries(libzippp PRIVATE libzip::libzip)
if (BUILD_SHARED_LIBS)
  target_compile_definitions(libzippp PRIVATE LIBZIPPP_EXPORTS)
endif()

if(LIBZIPPP_BUILD_TESTS)
  enable_testing()
  add_executable(libzippp_test "tests/tests.cpp")
  target_link_libraries(libzippp_test PRIVATE libzippp)
  add_test(NAME libzippp_tests COMMAND libzippp_test)
endif()

if(LIBZIPPP_INSTALL)
  install(
    TARGETS libzippp 
    EXPORT libzipppTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib 
    RUNTIME DESTINATION bin 
  )

  install(FILES src/libzippp.h DESTINATION include/libzippp)

  include(CMakePackageConfigHelpers)

  set(PROJECT_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/libzipppConfig.cmake")
  set(PROJECT_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/libzipppConfigVersion.cmake")

  configure_package_config_file(
    "Config.cmake.in" 
    ${PROJECT_CONFIG_FILE} 
    INSTALL_DESTINATION "share/libzippp"
  )
  write_basic_package_version_file(
    ${PROJECT_VERSION_FILE}
    COMPATIBILITY SameMajorVersion
  )

  install(
    FILES ${PROJECT_CONFIG_FILE} ${PROJECT_VERSION_FILE} cmake/FindLIBZIP.cmake
    DESTINATION "share/libzippp"
  )

  install(
    EXPORT libzipppTargets
    NAMESPACE "libzippp::"
    DESTINATION "share/libzippp"
  )
endif()

