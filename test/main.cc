#include "sargs.h"
#include <cassert>

using namespace sargs;
using namespace std;

void TestFlagParser() {
  std::string str1 = "program";
  std::string str2 = "-a";
  std::string str3 = "--bee=12";
  std::string str4 = "--";
  std::string str5 = "nonflag1";
  std::string str6 = "nonflag2";

  FlagParser parser;
  char* argv[6] = { &str1.front(), &str2.front(), &str3.front(), &str4.front(),
                    &str5.front(), &str6.front() };
  parser.Parse(6, argv);
  assert(parser.Has("-a"));
  assert(parser.Has("--bee"));
  assert(parser.Has("--bee"));

  std::vector<std::string> nonflags = parser.GetNonFlags();
  assert(nonflags.size() == 2);
  assert(nonflags[0] == "nonflag1");
  assert(nonflags[1] == "nonflag2");
}

void TestSarg1() {
  std::string str1 = "program";
  std::string str2 = "-c=12";
  std::string str3 = "--goof";
  std::string str4 = "--super=mfile.dat";

  char* argv[4] = { &str1.front(), &str2.front(), &str3.front(), &str4.front() };
  Args args;
  args.AddRequiredFlag("-c", "", "The dash c flag");
  args.AddRequiredFlag("--goof", "", "The goof flag");
  args.AddRequiredFlag("--super", "-s", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");
  args.Initialize(4, argv);

  std::string c_str = args.GetAsString("-c");
  assert(c_str == "12");

  int64_t c_int64 = args.GetAsInt64("-c");
  assert(c_int64 == 12);

  assert(args.Has("--goof"));
}

void TestAlias() {
  std::string str1 = "program";
  std::string str2 = "-c=12";
  std::string str3 = "--goof";
  std::string str4 = "--super=mfile.dat";

  char* argv[4] = { &str1.front(), &str2.front(), &str3.front(), &str4.front() };
  Args args;
  args.AddRequiredFlag("-c", "--thecflag", "The dash c flag");
  args.AddRequiredFlag("--goof", "", "The goof flag");
  args.AddRequiredFlag("--super", "-s", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");
  args.Initialize(4, argv);

  assert(args.Has("-c"));
  assert(args.Has("--thecflag"));
  assert(args.GetAsString("--thecflag") == "12");
}

void TestOptional() {
  std::string str1 = "program";
  std::string str2 = "-c=12";
  std::string str3 = "--goof";
  std::string str4 = "--";
  std::string str5 = "file1";

  char* argv[5] = { &str1.front(), &str2.front(), &str3.front(), &str4.front(), &str5.front() };

  Args args;
  args.AddRequiredFlag("-c", "--thecflag", "The dash c flag");
  args.AddRequiredFlag("--goof", "", "The goof flag");
  args.AddOptionalFlag("--super", "-s", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");
  args.RequireNonFlags(1);
  args.Initialize(5, argv);

  assert(args.Has("-c"));
  assert(args.Has("--thecflag"));
  assert(args.GetAsString("--thecflag") == "12");

  std::string value = "Dont change me";
  assert(args.GetAsString("--super", value) == false);
  assert(value == "Dont change me");
  args.SetEpilogue("\nSupport: webmaster@awesomesite.com\n\n");
  args.PrintUsage(cout);
}

int main(int argc, char* argv[]) {
  TestFlagParser();
  TestSarg1();
  TestAlias();
  TestOptional();
  return 0;
}

