#include <sargs.h>

using namespace std;

#include <sargs.h>

int main(int argc, char* argv[]) {
  SARGS_REQUIRED_FLAG_VALUE("--foo", "-f", "The description of foo");
  SARGS_OPTIONAL_FLAG("--bar", "", "The bar flag to help");
  SARGS_INITIALIZE(argc, argv);

  //
  // Run program code
  //

  return 0;
}
