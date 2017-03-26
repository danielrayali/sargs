# Simple Args

Simple Args (sargs) is a header-only C++ arguments parser. I wrote this so I would never have to integrate with a complicated thirdparty argument parser or write my own again. This parser is simple and has its pros and cons when compared to other libraries like gflags or Poco's argument parsing. It provides the bare minumum of features to keep it simple and integrate quickly into a new or existing project.

## Quick Start Guide

Sarg is header only, so just make sure it is available through your include directories. You need to put some code in your main function, or somwhere near there with access to the initial argument variables. These variables are not modified.

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

### Default Usage and Preamble

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

### Header Only

Just include sargs.h and you're good to go.

### Avilable Everywhere

Once Sarg has been initialized, the variables can be accessed through the static global singleton using the provided macros.

## Bugs/Comments

Please open github issues for this software or create a pull request if there is something that needs changing.

## API Documentation

```
// Initializes and validates the configuration for the arguments provided
// No return value. Can throw.
SARGS_INITIALIZE(argc, argv)

// Tell Sarg that a flag is required and requires no value to be specified
// Both flag and alias will be available for lookup once Sargs is initialized
// even if the user only specified one. description is used just for usage
// information. If a value is specified it is stored but ignored.
SARGS_REQUIRED_FLAG(flag, alias, description)

// Tell sargs that a flag is required and does require a value. Same behaviour as
// SARGS_REQUIRED_FLAG
SARGS_REQUIRED_FLAG_VALUE(flag, alias, description)

// Tell sargs that there might be an optional flag that may not have value.
SARGS_OPTIONAL_FLAG(flag, alias, description)

// Tell sargs that there might be an optional flag but it must have a value
SARGS_OPTIONAL_FLAG_VALUE(flag, alias, description)

// Replace the default preamble with a custom one
SARGS_SET_PREAMBLE(preamble)

// Set the epilogue
SARGS_SET_EPILOGUE(epilogue)

// Replace the default flag description string with a custom one
SARGS_SET_FLAG_DESCRIPTION(flag_description)

// Print the usage to the specified stream
SARGS_PRINT_USAGE(ostream)

// Print the usage to std::cout
SARGS_PRINT_USAGE_TO_COUT()

// Require that at least count non-flags are specified by the user
SARGS_REQUIRE_NONFLAGS(count)

// Get a non-flag based on the index it was specified by the user
SARGS_GET_NONFLAG(index)

// Get the value of a flag as an int64_t
SARGS_GET_INT64(flag)

// Get the value of a flag as a std::string
SARGS_GET_STRING(flag)

// Get the value of a flag as a float
SARGS_GET_FLOAT(flag)

// Return a bool of the flag was specified
SARGS_HAS(flag)

// Disable default -h and --help flags. These will do nothing if specified by
// the user when this is called before SARGS_INITIALIZE()
SARGS_DISABLE_HELP()

// Allow the argument validation to throw std::runtime_errors on the problems
// it finds. Description strings will be provided in the exceptions thrown.
SARGS_VALIDATION_THROWS()
```