#include "hello.h"
#include <iostream>
#include <sargs.h>

void Hello::operator()() {
  std::string what = SARGS_GET_STRING("--what");
  std::cout << "Hello, World: " << what << std::endl;
}
