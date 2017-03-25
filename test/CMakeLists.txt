if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wwrite-strings")
  add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

include_directories (${CMAKE_SOURCE_DIR}/src)

add_executable (sargs_test
  main.cc)

target_link_libraries (sargs_test)