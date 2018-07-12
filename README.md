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

```
./binary

Must specify --foo or -f

Usage:  <--bar|> <--help|-h> --foo=value 

  Required flags:
    --foo=value/-f=value      The description of foo

  Optional flags:
    --bar                     The bar flag to help
    --help/-h                 Print usage and options information
```

For more details on the full API, check the github [Wiki page](https://github.com/drali/sargs/wiki/API-Documentation).

## Flag Types

Flag are parsed as either optional or required, can require a value, or may be a "non-flag". Non-flags are unrecognized flags or anything encountered after the delimiter. The delimiter is two hyphens "--". This allows the command line user to specify flags in any order as long as they are specified in the source code. Flags are parsed in order of the index they are stored in the main()'s argv array. Each one is checked to ensure it is specified and expected. An example command line might be:

```./program <--optional_flag> --required_flag=3.14 -- nonflag```

Value flags require a value to be specified on the command line. You can specify values on these flags using equals or with just a space. Currently, it does not support non-spaced alpha-numeric flags, such as "-p2".

## Features

### Default Values for Value Flags

You can specify defaults for any flags that are specified with the ```*_VALUE_DEFAULT()``` APIs.

### Flag Aliasing

Flags can be specified with an alias. Both the flag and the alias are available through the normal getter interfaces, even if the command line user only specified one.

### Well Formatted Usage

A default usage message will be generated for you. This is broken down into the preamble, the flag description and the epilogue. The preamble is printed before the flag descriptions and the epilogue is printed after the flag descriptions. By default there is no epilogue. The default preamble is a basic usage example. The flag description describes all required and optional flags. For example:

```
Usage: ./program <--bar|--help|-h> --foo=value

  Required flags:
    --foo=value/-f=value      The description of foo

  Optional flags:
    --bar                     The bar flag to help
    --help/-h                 Print usage and options information
```

### Disable Defaults

Some default behaviors are configurable and able to be disabled. Below are the behaviors and the corresponding macros to change or disable it.

- Disable printing usage to cout on a parse error: ```SARGS_DISABLE_USAGE()```
- DIsable calling std::exit() on a parse error: ```SARGS_DISABLE_EXIT()```
- Replace default flag description string: ```SARGS_SET_USAGE()```
- Replace default preamble: ```SARGS_SET_PREAMBLE()```
- Set an epilogue: ```SARGS_SET_EPILOGUE()```
- Disable predefined ```--help```/```-h``` flags: ```SARGS_DISABLE_HELP()```
- Disable exceptions: ```SARGS_DISABLE_EXCEPTIONS()```

### Basic Value Conversions

Internally each argument value is kept as a string. These can be converted to a limited number of other types for you with proper error handling. If it can not be converted (e.g. Trying to get "abc" as a float) a ```std::runtime_error``` will be thrown. If no value is specified, ```SARGS_INITIALIZE()``` will print usage and exit or throw. These macros will return the value. For optional values, use ```SARGS_HAS()``` before converting the value or a SargsError may be thrown if the flag was not specified.

```
SARGS_GET_INT64(flag)
SARGS_GET_STRING(flag)
SARGS_GET_FLOAT(flag)
```

### Exceptions

By default, errors encountered during initialization will print usage and exit with a return code of zero. When calling one of the value conversion functions, it will throw if a flag or flag value are not specified on the command line and no default was set.

### Non-Flags

Use ```SARGS_REQUIRE_NONFLAGS()``` to ensure the user is required to set a specific number of non-flags. These can be iterated over the vector of strings returned by ```SARGS_GET_NONFLAGS()``` or accessed by the index it was specified with ```SARGS_GET_NONFLAG(index)```.

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
