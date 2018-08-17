#include "sargs.h"
#include <stdexcept>

using namespace sargs;
using namespace std;

static inline bool AssertFailed(const std::string& message) {
    cerr << "fail" << endl; \
    throw std::runtime_error(message);
}

#define Assert(x) ((x) || (AssertFailed(#x)))

void TestUnsigned() {
  cout << "TestUnsigned()...";

  string str1 = "program";
  string str2 = "-c=8446744073709551615";

  char* argv[2] = { &str1.front(), &str2.front() };
  Args args;
  args.AddRequiredFlagValue("-c", "", "The dash c flag");

  try { args.Initialize(2, argv); }
  catch (SargsError& error) {
    cerr << "fail" << endl;
    throw;
  }

  string c_str = args.GetAsString("-c");
  Assert(c_str == "8446744073709551615");
  Assert(args.GetAsUInt64("-c") == 8446744073709551615);
  try {
    args.GetAsUInt32("-c");
    cerr << "fail" << endl;
    throw std::runtime_error("GetAsUInt32 failed");
  } catch (SargsError& error) {}
  try {
    args.GetAsUInt16("-c");
    cerr << "fail" << endl;
    throw std::runtime_error("GetAsUInt16 failed");
  } catch (SargsError& error) {}
  try {
    args.GetAsUInt8("-c");
    cerr << "fail" << endl;
    throw std::runtime_error("GetAsUInt8 failed");
  } catch (SargsError& error) {}

  cout << "pass" << endl;
}

void TestValues() {
  cout << "TestValues()...";

  std::string str1 = "program";
  std::string str2 = "-c=12";
  std::string str3 = "--super=mfile.dat";

  char* argv[3] = { &str1.front(), &str2.front(), &str3.front() };
  Args args;
  args.AddRequiredFlagValue("-c", "", "The dash c flag");
  args.AddRequiredFlagValue("--super", "", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");

  try { args.Initialize(3, argv); }
  catch (SargsError& er) {
    cerr << "fail" << endl;
    throw;
  }

  std::string c_str = args.GetAsString("-c");
  Assert(c_str == "12");

  int64_t c_int64 = args.GetAsInt64("-c");
  Assert(c_int64 == 12);

  Assert(args.Has("--super"));

  cout << "pass" << endl;
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
    cerr << "fail" << endl;
    throw;
  }

  Assert(args.Has("-c"));
  Assert(args.Has("--thecflag"));
  Assert(args.GetAsString("--thecflag") == "12");
  Assert(args.Has("--super"));
  Assert(args.Has("-s"));

  cout << "pass" << endl;
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

  try { args.Initialize(5, argv); }
  catch (SargsError& ex) {
    cerr << "fail" << endl;
    throw;
  }

  Assert(args.Has("-c"));
  Assert(args.Has("--thecflag"));
  Assert(args.GetAsString("--thecflag") == "12");
  Assert(args.GetAsString("-c") == "12");

  std::string value = "Dont change me";
  Assert(args.GetAsString("--super", value) == false);
  Assert(value == "Dont change me");
  Assert(args.GetAsString("-s", value) == false);
  Assert(value == "Dont change me");

  cout << "pass" << endl;
}

void TestNonFlags() {
  cout << "TestNonFlags()...";

  std::string str1 = "program";
  std::string str2 = "--display";
  std::string str3 = "./file1";
  std::string str4 = "filetwo";

  char* argv[4] = { &str1.front(), &str2.front(), &str3.front(), &str4.front() };

  Args args;
  args.AddOptionalFlag("--display", "-d", "Displays");
  args.AddOptionalFlag("--convert", "-c", "Converts");
  args.RequireNonFlags(2);

  try { args.Initialize(4, argv); }
  catch (SargsError& ex) {
    cerr << "fail" << endl;
    throw;
  }

  Assert(args.GetNonFlag(0) == "./file1");
  Assert(args.GetNonFlag(1) == "filetwo");

  cout << "pass" << endl;
}

void TestFallback() {
  cout << "TestFallback()...";

  std::string str1 = "program";
  std::string str2 = "--goof";
  std::string str3 = "--";
  std::string str4 = "file1";

  char* argv[4] = { &str1.front(), &str2.front(), &str3.front(), &str4.front() };

  Args args;
  args.AddOptionalFlagValue("-c", "--thecflag", "The dash c flag", "24");
  args.AddRequiredFlag("--goof", "", "The goof flag");
  args.AddOptionalFlagValue("--super", "-s", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras id nulla dapibus, rutrum odio id, posuere libero. Fusce pretium tellus porttitor ex rhoncus, nec consectetur eros auctor. Sed libero sem, sodales et ultrices eu, interdum sit amet orci. Donec pulvinar sapien ut velit lacinia porta. Phasellus porttitor vehicula dolor, nec egestas dui sagittis in. Vivamus facilisis, diam a consectetur fermentum, augue quam egestas purus, interdum faucibus ex velit non lorem. Ut id aliquet leo. Aenean porta neque non sapien faucibus, nec bibendum lacus egestas. Fusce malesuada dolor sagittis mauris mollis, eu varius lacus consequat. Donec mauris metus, tincidunt quis diam ac, malesuada molestie orci. Nunc non dolor rhoncus, hendrerit augue sed, vulputate ante. Cras leo lacus, sollicitudin vitae sem accumsan, commodo tristique ante. Morbi interdum ligula vitae sem rhoncus blandit.");
  args.RequireNonFlags(1);

  try { args.Initialize(4, argv); }
  catch (SargsError& ex) {
    cerr << "fail" << endl;
    throw;
  }

  Assert(args.Has("-c"));
  Assert(args.Has("--thecflag"));
  Assert(args.GetAsString("--thecflag") == "24");
  Assert(args.GetAsString("-c") == "24");

  cout << "pass" << endl;
}

int main(int argc, char* argv[]) {
  TestValues();
  TestAlias();
  TestOptional();
  TestNonFlags();
  TestFallback();
  TestUnsigned();

  printf("\nTests complete\n");
  return 0;
}
