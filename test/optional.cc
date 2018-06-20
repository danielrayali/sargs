#include <sargs.h>

using namespace std;
using namespace sargs;

int main(int argc, char* argv[]) {
  SARGS_OPTIONAL_FLAG_VALUE_DEFAULT("--display", "-d", "Displays", "1024x2048");
  SARGS_OPTIONAL_FLAG_VALUE_DEFAULT("--convert", "-c", "Converts", "standard");
  try { SARGS_INITIALIZE(argc, argv); }
  catch (SargsError& er) {
    cerr << "Sargs threw error" << endl;
    return 1;
  }
  catch (SargsUsage& usage) {
    cout << "Sargs threw usage" << endl;
    return 1;
  }

  cout << "Display is " << SARGS_GET_STRING("--display") << endl;
  cout << "Convert is " << SARGS_GET_STRING("--convert") << endl;

  return 0;
}
