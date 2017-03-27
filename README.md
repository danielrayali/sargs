# Simple Args

Simple Args (sargs) is a header-only C++ arguments parser. I wrote this so I would never have to integrate with a complicated thirdparty argument parser or write my own again. This parser is simple and has its pros and cons when compared to other libraries like gflags or Poco's argument parsing. It provides the bare minumum of features to keep it simple and integrate quickly into a new or existing project.

## Quick Start Guide

sargs is header only, so just make sure it is available through your include directories. You need to put some code in your main function, or somwhere near there with access to the initial argument variables. These variables are not modified.

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

## Installation

Just include sargs.h in your include directories and you're good to go.

## Argument Format

Sargs expects certain conditions from the argument format. The general assembly of arguments is:

```./program <--optional|--flags> --required --flags -- nonflag1 nonflag2```

Optional flags and required flags need not be in a particular order. These must however come before non-flags. Non-flags can be designated once the separator (```--```) is optionally specified on the command line. Notice the separation between ```--``` and ```nonflag1```.

Sargs expects flags to start with a hyphen (```-```). You can specify values on these flags using equals or with just a space.

```./program --flag1=value1 --flag2 value2```

During flag parsing, values will be associated with flags until the separator is encountered. If the separator is not specified Sargs will assume the first non-flag, non-value argument it encounters is the start of the non-flags. For example

```./program --flag1 value1 nonflag1```

has one flag (```--flag1```) with the associated value (```value1```) and one nonflag (```nonflag1```).

## Features

### Required Arguments

If you have flags that must be specified to run correctly you can specify them with ```SARGS_REQUIRED_FLAG```. If there needs to be a value associated with the flag use ```SARGS_REQUIRED_FLAG_VALUE```. If this value is not specified usage information will be displayed by default. If you would like ```SARGS_INITIALIZE()``` to throw a ```std::runtime_error``` instead, use ```SARGS_THROW_ON_VALIDATION()```.

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

### Non-Zero Non-Flags Can Be Required

Use ```SARGS_REQUIRE_NONFLAGS()``` to ensure the user is required to set a specific number of non-flags. These can be iterated over the vector of strings returned by ```SARGS_GET_NONFLAGS()```.

### Argument Specification Not Required

If you want to get started fast and don't care about Usage, you can just initialize sargs and start accessing flags as needed, where they are needed. Optional arguments exist only to provide context to the Usage information.

### Header Only

Just include sargs.h and you're good to go.

### Avilable Everywhere

Once sargs has been initialized, the variables can be accessed through the static global singleton using the provided macros.

## Bugs/Comments

Please open github issues for this software or create a pull request if there is something that needs changing.

## License

Copyright (c) 2017 drali. All rights reserved.

This software is provided 'as-is', without any express or implied warranty.
In no event will the author be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial
applications, and to alter it and redistribute it freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
 2. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 3. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
