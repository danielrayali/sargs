#include "sargs.h"
#include <cassert>

using namespace sargs;
using namespace std;

void TestSarg1() {
  cout << "TestSarg1()...";

  std::string str1 = "program";
  std::string str2 = "-c=12";
  std::string str3 = "--goof";
  std::string str4 = "--super=mfile.dat";

  char* argv[4] = { &str1.front(), &str2.front(), &str3.front(), &str4.front() };
  Args args;
  args.AddRequiredFlagValue("-c", "", "The dash c flag");
  args.AddRequiredFlag("--goof", "", "The goof flag");
  args.AddRequiredFlagValue("--super", "-s", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");
  args.DisableExit();

  try { args.Initialize(4, argv); }
  catch (SargsError& er) {
    printf("Sargs error TestSarg1\n");
  }

  std::string c_str = args.GetAsString("-c");
  assert(c_str == "12");

  int64_t c_int64 = args.GetAsInt64("-c");
  assert(c_int64 == 12);

  assert(args.Has("--goof"));

  cout << "done" << endl;
}

void TestAlias() {
  cout << "TestAlias()...";

  std::string str1 = "program";
  std::string str2 = "-c=12";
  std::string str3 = "--goof";
  std::string str4 = "--super=mfile.dat";

  char* argv[4] = { &str1.front(), &str2.front(), &str3.front(), &str4.front() };
  Args args;
  args.AddRequiredFlagValue("-c", "--thecflag", "The dash c flag");
  args.AddRequiredFlag("--goof", "", "The goof flag");
  args.AddRequiredFlagValue("--super", "-s", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");

  try { args.Initialize(4, argv); }
  catch (SargsError& er) {
    printf("Sargs error\n");
  }

  assert(args.Has("-c"));
  assert(args.Has("--thecflag"));
  assert(args.GetAsString("--thecflag") == "12");

  cout << "done" << endl;
}

void TestOptional() {
  cout << "TestOptional()...";

  std::string str1 = "program";
  std::string str2 = "-c=12";
  std::string str3 = "--goof";
  std::string str4 = "--";
  std::string str5 = "file1";

  char* argv[5] = { &str1.front(), &str2.front(), &str3.front(), &str4.front(), &str5.front() };

  Args args;
  args.AddRequiredFlagValue("-c", "--thecflag", "The dash c flag");
  args.AddRequiredFlag("--goof", "", "The goof flag");
  args.AddOptionalFlagValue("--super", "-s", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");
  args.RequireNonFlags(1);
  args.DisableExceptions();
  try { args.Initialize(5, argv); }
  catch (SargsError& ex) {
    printf("Sargs threw\n");
  }

  assert(args.Has("-c"));
  assert(args.Has("--thecflag"));
  assert(args.GetAsString("--thecflag") == "12");

  std::string value = "Dont change me";
  assert(args.GetAsString("--super", value) == false);
  assert(value == "Dont change me");
  args.SetEpilogue("\nSupport: webmaster@awesomesite.com\n\n");

  cout << "done" << endl;
}

void TestHelp() {
  cout << "TestHelp()...";

  std::string str1 = "program";
  std::string str2 = "-c=12";
  std::string str3 = "--garbage";
  std::string str4 = "--";
  std::string str5 = "file1";

  char* argv[5] = { &str1.front(), &str2.front(), &str3.front(), &str4.front(), &str5.front() };

  Args args;
  args.AddOptionalFlag("--display", "-d", "Displays");
  args.AddOptionalFlagValue("--convert", "-c", "Converts");
  args.RequireNonFlags(1);
  args.DisableExit();
  args.DisableUsage();
  try { args.Initialize(5, argv); }
  catch (SargsError& ex) {
    printf("Sargs threw\n");
  }

  cout << "done" << endl;
}

void TestNonFlags() {
  cout << "TestNonFlags()...";

  std::string str1 = "program";
  std::string str2 = "--display";
  std::string str3 = "./file1";

  char* argv[3] = { &str1.front(), &str2.front(), &str3.front() };

  Args args;
  args.AddOptionalFlag("--display", "-d", "Displays");
  args.AddOptionalFlag("--convert", "-c", "Converts");
  args.RequireNonFlags(1);
  try { args.Initialize(3, argv); }
  catch (SargsError& ex) {
    printf("Sargs threw\n");
    args.PrintUsage(cout);
  }

  assert(args.GetNonFlag(0) == "./file1");

  cout << "done" << endl;
}

int main(int argc, char* argv[]) {
  TestSarg1();
  TestAlias();
  TestOptional();
  TestHelp();
  TestNonFlags();

  printf("\nTests complete\n");
  return 0;
}

