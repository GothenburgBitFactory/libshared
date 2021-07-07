cmake_minimum_required (VERSION 3.1)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set (HAVE_CMAKE true)
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 17)

project (shared)
include (CXXSniffer)

include (CheckFunctionExists)

set (PROJECT_VERSION "1.0.0")

set (PACKAGE "${PROJECT_NAME}")
set (VERSION "${PROJECT_VERSION}")
set (PACKAGE_BUGREPORT "support@taskwarrior.org")
set (PACKAGE_NAME "${PACKAGE}")
set (PACKAGE_TARNAME "${PACKAGE}")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_STRING "${PACKAGE} ${VERSION}")

check_function_exists(strlcpy HAVE_STRLCPY)

message ("-- Configuring cmake.h")
configure_file (
  ${CMAKE_SOURCE_DIR}/cmake.h.in
  ${CMAKE_SOURCE_DIR}/cmake.h)

add_subdirectory (src)
if (EXISTS ${CMAKE_SOURCE_DIR}/test)
  add_subdirectory (test EXCLUDE_FROM_ALL)
endif (EXISTS ${CMAKE_SOURCE_DIR}/test)
