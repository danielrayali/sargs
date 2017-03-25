#include "hello.h"
#include <iostream>
#include <sarg.h>

void Hello::operator()() {
  std::string what = SARG_GET_STRING("--what");
  std::cout << "Hello, World: " << what << std::endl;
}
