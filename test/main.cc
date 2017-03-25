#include "sarg.h"
#include "flag_parser.h"
#include <cassert>

using namespace sarg;

void TestFlagParser() {
  FlagParser parser;
  char* argv[6] = { "program", "-a", "--bee=12", "--", "nonflag1", "nonflag2" };
  parser.Parse(6, argv);
  assert(parser.Has("-a"));
  assert(parser.Has("--bee"));
  assert(parser.Has("--bee"));

  std::vector<std::string> nonflags = parser.GetNonFlags();
  assert(nonflags.size() == 2);
  assert(nonflags[0] == "nonflag1");
  assert(nonflags[1] == "nonflag2");
}

void TestSarg() {
  char* argv[4] = { "program", "-c=12", "--goof", "--super=mfile.dat"};
  SARG_REQUIRED_FLAG("-c", "", "The dash c flag");
  SARG_REQUIRED_FLAG("--goof", "", "The goof flag");
  SARG_REQUIRED_FLAG("--super", "-s", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");
  SARG_INITIALIZE(4, argv);

  std::string c_str = SARG_GET_STRING("-c");
  assert(c_str == "12");

  int64_t c_int64 = SARG_GET_INT64("-c");
  assert(c_int64 == 12);

  assert(SARG_HAS("--goof"));

  SARG_PRINT_USAGE_TO_COUT();
}

int main(int argc, char* argv[]) {
  TestFlagParser();
  TestSarg();
  return 0;
}

