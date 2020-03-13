cmake_minimum_required(VERSION 3.16)
project(HHMCoordinateConverter)

set(CMAKE_CXX_STANDARD          20  CACHE STRING "CXX standard version")
set(CMAKE_CXX_STANDARD_REQUIRED ON  CACHE BOOL   "version required")
set(CMAKE_CXX_EXTENSIONS        OFF CACHE BOOL   "compiler extension")

find_package(Eigen3  REQUIRED)

set(Clang_WARN_OPTIONS
  -Weverything
  -Wno-c++98-compat
)
set(GNU_WARN_OPTIONS
  -Wall
  -Walloc-zero
  -Walloca
  -Wcast-qual
  -Wconditionally-supported
  -Wconversion
  -Wdate-time
  -Wdisabled-optimization
  -Wdouble-promotion
  -Wduplicated-branches
  -Wduplicated-cond
  -Wextra
  -Whsa
  -Wimplicit-fallthrough
  -Winvalid-pch
  -Wlogical-op
  -Wmissing-declarations
  -Wmissing-include-dirs
  -Wnormalized
  -Wnull-dereference
  -Wopenmp-simd
  -Wpacked
  -Wpedantic
  -Wredundant-decls
  -Wrestrict
  -Wsized-deallocation
  -Wstack-protector
  -Wstrict-overflow
  -Wsuggest-attribute=noreturn
  -Wsuggest-final-methods
  -Wsuggest-final-types
  -Wswitch-enum
  -Wsync-nand
  -Wtrampolines -Wfloat-equal -Wshadow
  -Wuninitialized
  -Wunsafe-loop-optimizations
  -Wunused-parameter
  -Wuseless-cast
  -Wvla
  -Wzero-as-null-pointer-constant
  --pedantic-errors
)

add_compile_options(
  "$<$<CXX_COMPILER_ID:Clang>:${Clang_WARN_OPTIONS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARN_OPTIONS}>"
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

add_executable(converter src/main.cpp)
target_link_libraries(converter Eigen3::Eigen)

add_custom_target(run
  COMMAND converter < ${CMAKE_SOURCE_DIR}/input
  DEPENDS converter
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# For WSL clipboard
add_custom_target(clip
  DEPENDS converter
  COMMAND clip.exe < ${CMAKE_SOURCE_DIR}/src/main.cpp)
