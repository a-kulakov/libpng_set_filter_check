cmake_minimum_required(VERSION 3.1) # libpng16
set(MODULE_NAME set_filter_check)

project(${MODULE_NAME} C)

option(PNG_SHARED "Build shared lib" OFF)
option(PNG_STATIC "Build static lib" ON)
option(PNG_TESTS  "Build libpng tests" OFF)

# LIBPNG_SRC_DIR - source dir with libpng library code
set(LIBPNG_SRC_DIR "../libpng" CACHE PATH "Path to libpng sources")
set(LIBPNG_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/libpng")
add_subdirectory("${LIBPNG_SRC_DIR}" "${LIBPNG_BIN_DIR}")

add_executable(${MODULE_NAME}
  set_filter_check.c
)

if (TARGET png12_static)
  add_library(png_static ALIAS png12_static)
  set(EXTRA_LIBS z)
endif()

target_link_libraries(${MODULE_NAME}
  png_static
  ${EXTRA_LIBS}
)

target_include_directories(${MODULE_NAME}
  PRIVATE
    $<TARGET_PROPERTY:png_static,INCLUDE_DIRECTORIES>
    $<BUILD_INTERFACE:${LIBPNG_BIN_DIR}> # pnglibconf.h
)
