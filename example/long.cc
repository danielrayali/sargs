#include <sargs.h>

using namespace std;

#include <sargs.h>

int main(int argc, char* argv[]) {
  SARGS_REQUIRED_FLAG_VALUE("--foo", "-f", "The description of foo");
  SARGS_OPTIONAL_FLAG("--thisisaverylongflagthatwillchangewherethedescriptionstarts", "", "The bar flag to help");
  SARGS_OPTIONAL_FLAG("--this", "", "This is a very long description intended to test where the line will wrap for the description of this particular flag.");

  // This will move the start column of the flag description further to the right. Rather than trying to figure it out,
  // just let the user set it.
  SARGS_SET_DESC_START_COLUMN(67);
  SARGS_SET_DESC_WIDTH(60);
  SARGS_INITIALIZE(argc, argv);

  //
  // Run program code
  //

  return 0;
}
