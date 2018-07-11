# Simple Args

Simple Args (sargs) is a header-only C++ arguments parser. I wrote this so I would never have to integrate with a complicated thirdparty argument parser or write my own again. It attempts to provide the bare minumum of features to keep it simple and integrate quickly into a new or existing project.

## Quick Start Guide

sargs is header only, so just make sure it is available through your include directories. You need to put some code in your main function, or somwhere near there with access to the initial argument variables (argc and argv). These variables (argc and argv) are not modified.

```cpp
//
// main.cc
//

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
```

```cpp
//
// worker.cc
//

Worker::Worker() {
  std::string foo = SARGS_GET_STRING("--foo");
  bool bar = SARGS_HAS("--bar");
}
```

For more details on the full API, check the github [Wiki page](https://github.com/drali/sargs/wiki/API-Documentation).

## Argument Format

Sargs allows the user to specify optional and required flags.  values or no values. There is also a concept of "non-flags". Non-flags are any unrecognized flags encountered, or ones after the "--" delimiter. During flag parsing, flags are parsed in order of the index they are specified in the main's argv array. Each one is checked to ensure it is specified and expected. The general assembly of arguments is:

```./program <--optional|--flags> --required=3.14 --flags -- nonflag1 nonflag2```

Optional flags, required flags, and non-flags need not be in a particular order. Non-flags are assumed once the separator (```--```) is encountered.

Value flags require a value to be specified on the command line. You can specify values on these flags using equals or with just a space. Currently, it does not support non-spaced alpha-numeric flags, such as "-p2".

```./program --flag1=value1 --flag2 value2``` 

## Features

### Disabling Usage and Exit

By default, if an unexpected flag is encountered or the wrong number of non-flags usage will be printed to std::cout and call std::exit() with the value of zero. Sargs allows the user to disable the printing of usage to std::cout if an error is encountered with ```SARGS_DISABLE_USAGE()```. Sargs also allows you to disable the call to std::exit() with ```SARGS_DISABLE_EXIT()```.

### Required Argument

If you have flags that must be specified to run correctly you can specify them with ```SARGS_REQUIRED_FLAG```. If there needs to be a value associated with the flag use ```SARGS_REQUIRED_FLAG_VALUE```. If this value is not specified usage will be displayed and std::exit will be called.

### Default Flag Description and Preamble

A default preamble and flag description will be generated for you. The preamble is printed before the flag descriptions and the epilogue is printed after the flag descriptions. By default there is no epilogue. The default preamble is a basic usage format. Below is the preamble and flag description from the Quick Start section.

```
Usage: ./program <--bar|--help|-h> --foo=value

  Required flags:
    --foo=value/-f=value      The description of foo

  Optional flags:
    --bar                     The bar flag to help
    --help/-h                 Print usage and options information
```

### Disable Defaults

There are a few things that are default behaviour. Below is a list of these default behaviors and the corresponding macro to change or disable it.

- Replace default flag description string: ```SARGS_SET_USAGE()```
- Replace default preamble: ```SARGS_SET_PREAMBLE()```
- Set an epilogue: ```SARGS_SET_EPILOGUE()```
- Throw during validation instead of printing usage and exiting: ```SARGS_VALIDATION_THROWS()```
- Disable predefined ```--help```/```-h``` flags and behavior: ```SARGS_DISABLE_HELP()```.

### Basic Value Conversions

Internally each argument value is kept as a string. These can be converted to a limited number of other types for you with proper error handling. If it can not be converted (e.g. Trying to get "abc" as a float) a ```std::runtime_error``` will be thrown. If no value is specified, ```SARGS_INITIALIZE()``` will print usage and exit or throw. These macros will return the value. For optional values, use ```SARGS_HAS()``` before converting the value.

```
SARGS_GET_INT64(flag)
SARGS_GET_STRING(flag)
SARGS_GET_FLOAT(flag)
```
### Exceptions

By default, errors encountered during initialization will print usage and exit with a return code of zero. When calling one of the value conversion functions, it will throw if a flag's value was not specified or if the flag wasn't specified at all. Because parsing happens at initialization, this means the flag must be optional and require a value to get to a value conversion call.

### Non-Flags

Use ```SARGS_REQUIRE_NONFLAGS()``` to ensure the user is required to set a specific number of non-flags. These can be iterated over the vector of strings returned by ```SARGS_GET_NONFLAGS()``` or accessed by the index it was specified with ```SARGS_GET_NONFLAG(index)```.

### Aliases

When specifing flags, Sargs allows the user to specify an alias as well. All values with flags and aliases can be accessed using either string with the documented getters.

## Bugs/Comments

Please open github issues for this software or create a pull request if there is something that needs changing. All are welcome!

## License

Copyright (c) 2017 drali. All rights reserved.

This software is provided 'as-is', without any express or implied warranty.
In no event will the author be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial
applications, and to alter it and redistribute it freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
 2. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 3. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
