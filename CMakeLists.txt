cmake_minimum_required (VERSION 2.8)

project (SimpleArgs)

#
# Common compiler flags
#

# Print commands when executing
set(CMAKE_VERBOSE_MAKEFILE on)

#
# Compiler flags
#

if(MSVC)
  # Silence a compiler message about Windows
  add_definitions (-D_WIN32_WINNT=0x0501)
  set(CMAKE_CXX_FLAGS_DEBUG           "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL      "/MT /O1 /Ob1 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE         "/MT /O2 /Ob2 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "/MT /Zi /O2 /Ob1 /D NDEBUG")
else ()
  # Assuming gcc/g++ compiler flags
  set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall -Wextra -pedantic")
  if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
  endif ()
endif ()

include_directories (src)

#
# Testing setup
#

add_subdirectory (test)
add_subdirectory (example)
