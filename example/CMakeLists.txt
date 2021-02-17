if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wwrite-strings")
  add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

include_directories (${CMAKE_SOURCE_DIR}/src)

add_executable (example
  main.cc
  hello.cc
  hello.h)

target_link_libraries (example)

add_executable (simple simple.cc)
target_link_libraries (simple)

add_executable (optional optional.cc)
target_link_libraries (optional)

add_executable (long long.cc)
target_link_libraries (long)
