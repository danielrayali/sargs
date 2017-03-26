#include <sargs.h>
#include "hello.h"

int main(int argc, char* argv[]) {
  SARGS_REQUIRED_FLAG_VALUE("--what", "-w", "The thing to say");
  SARGS_REQUIRED_FLAG("--other", "", "The other flag thing here. This description is probably longer than 40 characters");
  SARGS_DISABLE_HELP();
  SARGS_INITIALIZE(argc, argv);
  Hello hello;
  hello();
  return 0;
}
