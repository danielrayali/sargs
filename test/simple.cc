#include <sargs.h>

using namespace std;

int main(int argc, char* argv[]) {
  SARGS_OPTIONAL_FLAG("--display", "-d", "Displays");
  SARGS_OPTIONAL_FLAG("--convert", "-c", "Converts");
  SARGS_REQUIRE_NONFLAGS(1);
  SARGS_INITIALIZE(argc, argv);

  cout << "Hello, world!" << endl;
  return 0;
}
