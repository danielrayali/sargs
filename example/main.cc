#include <sarg.h>
#include "hello.h"

int main(int argc, char* argv[]) {
  SARG_REQUIRED_FLAG_VALUE("--what", "-w", "The thing to say");
  SARG_REQUIRED_FLAG("--other", "", "The other flag thing here. This description is probably longer than 40 characters");
  SARG_DISABLE_HELP();
  SARG_INITIALIZE(argc, argv);
  Hello hello;
  hello();
  return 0;
}
