/*
 * Copyright (c) 2017 drali. All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the author be held liable for any damages arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
 *  2. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 *  3. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 */
#pragma once

#include <cerrno>
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace sargs {

class SargsError : public std::runtime_error {
 public:
  SargsError(const std::string& message) : std::runtime_error(message) {};
  ~SargsError() = default;
};

struct Argument {
  Argument(const std::string& _flag,
           const std::string& _alias,
           const std::string& _description,
           const bool _value) :
    flag(_flag), alias(_alias), description(_description), value(_value) {}

  Argument(const std::string& _flag,
           const std::string& _alias,
           const std::string& _description,
           const bool _value,
           const std::string& _fallback) :
    flag(_flag), alias(_alias), description(_description), fallback(_fallback), value(_value) {}

  std::string flag;
  std::string alias;
  std::string description;
  std::string fallback;
  bool value = false;
};

class Args {
 public:
  Args() = default;

  ~Args() = default;

  static Args& Default() {
    static Args instance;
    return instance;
  }

  void Initialize(int argc, char* argv[]) {
    if (_help_enabled)
      this->AddOptionalFlag("--help", "-h", "Print usage and options information");

    std::string result = this->Parse(argc, argv);
    this->GenerateUsage();
    const bool help_specified = this->Has("--help") || this->Has("-h");
    const bool usage = (_help_enabled && help_specified) || !result.empty();

    if (usage) {
      if (_usage_enabled) {
        if (!result.empty())
          std::cout << "\n" << result << "\n"  << std::endl;
        this->PrintUsage(std::cout);
      }

      if (_exit_enabled)
        exit(0);
    }

    this->AddFallbackValues();
  }

  bool GetAsString(const std::string& flag, std::string& value) const {
    if (flag.empty()) {
      if (_exceptions_enabled)
        throw SargsError("Flag query empty");
      else
        return false;
    }

    auto iter = _arguments.find(flag);
    if (iter == _arguments.end())
      return false;
    value = iter->second;
    return true;
  }

  std::string GetAsString(const std::string& flag) const {
    std::string value;
    if (!this->GetAsString(flag, value))
      return "";
    return value;
  }

  bool GetAsFloat(const std::string& flag, float& value) const {
    if (flag.empty()) {
      if (_exceptions_enabled)
        throw SargsError("Flag query empty");
      else
        return false;
    }

    auto iter = _arguments.find(flag);
    if (iter == _arguments.end())
      return false;

    const std::string value_str(iter->second);
    const float myvalue = std::strtof(value_str.c_str(), nullptr);
    if (errno == ERANGE)
      throw SargsError("Could not convert " +  value_str + " to float");

    value = myvalue;
    return true;
  }

  float GetAsFloat(const std::string& flag) const {
    float value;
    if (!this->GetAsFloat(flag, value))
      return 0.0;
    return value;
  }

  bool GetAsInt64(const std::string& flag, int64_t& value) const {
    if (flag.empty()) {
      if (_exceptions_enabled)
        throw SargsError("Flag query empty");
      else
        return false;
    }

    auto iter = _arguments.find(flag);
    if (iter == _arguments.end())
      return false;

    const std::string value_str(iter->second);
    const int64_t myvalue = std::strtol(value_str.c_str(), nullptr, 0);
    const bool range_error = (errno == ERANGE);
    if (range_error || (myvalue == 0 && flag != "0")) {
      if (_exceptions_enabled)
        throw SargsError("Could not convert " +  value_str + " to int64_t");
      else
        return false;
    }

    value = myvalue;
    return true;
  }

  int64_t GetAsInt64(const std::string& flag) const {
    int64_t value;
    if (!this->GetAsInt64(flag, value))
      if (_exceptions_enabled)
        throw SargsError(flag + " was not specified");
      else
        return 0;
    return value;
  }

  int32_t GetAsInt32(const std::string& flag) const {
    int64_t value;
    if (!this->GetAsInt64(flag, value))
      if (_exceptions_enabled)
        throw SargsError(flag + " was not specified");
      else
        return 0;
    return static_cast<int32_t>(value);
  }

  int32_t GetAsInt8(const std::string& flag) const {
    int64_t value;
    if (!this->GetAsInt64(flag, value))
      if (_exceptions_enabled)
        throw SargsError(flag + " was not specified");
      else
        return 0;
    return static_cast<int8_t>(value);
  }

  bool Has(const std::string& flag) const {
    return _arguments.find(flag) != _arguments.end();
  }

  std::string GetNonFlag(const size_t index) const {
    return _nonflags.at(index);
  }

  std::vector<std::string> GetNonFlags() const {
    return _nonflags;
  }

  void AddRequiredFlag(const std::string& flag, const std::string& alias, const std::string& description) {
    _required.emplace_back(flag, alias, description, false);
  }

 void AddRequiredFlagValue(const std::string& flag, const std::string& alias, const std::string& description) {
    _required.emplace_back(flag, alias, description, true, "");
 }

  void AddRequiredFlagValue(const std::string& flag, const std::string& alias, const std::string& description,
                            const std::string& fallback) {
    _required.emplace_back(flag, alias, description, true, fallback);
  }

  void AddOptionalFlag(const std::string& flag, const std::string& alias, const std::string& description) {
    _optional.emplace_back(flag, alias, description, false);
  }

  void AddOptionalFlagValue(const std::string& flag, const std::string& alias, const std::string& description) {
    _optional.emplace_back(flag, alias, description, true, "");
  }

  void AddOptionalFlagValue(const std::string& flag, const std::string& alias, const std::string& description,
                            const std::string& fallback) {
    _optional.emplace_back(flag, alias, description, true, fallback);
  }

  void RequireNonFlags(const int count) {
    _nonflags_required = count;
  }

  void PrintUsage(std::ostream& output) {
    output << _preamble << _flag_description << _epilogue;
  }

  void SetPreamble(const std::string& preamble) {
    _preamble = preamble;
  }

  void SetEpilogue(const std::string& epilogue) {
    _epilogue = epilogue;
  }

  void SetFlagDescription(const std::string& flag_description) {
    _flag_description = flag_description;
  }

  void DisableHelp() {
    _help_enabled = false;
  }

  void DisableExit() {
    _exit_enabled = false;
  }

  void DisableExceptions() {
    _exceptions_enabled = false;
  }

  void DisableUsage() {
    _usage_enabled = false;
  }

  std::string GetPreamble() const {
    return _preamble;
  }

  std::string GetEpilogue() const {
    return _epilogue;
  }

  std::string GetFlagDescription() const {
    return _flag_description;
  }

  std::string GetBinary() const {
    return _binary;
  }

 private:
  std::vector<Argument> _required;
  std::vector<Argument> _optional;
  std::map<std::string, std::string> _arguments;
  std::vector<std::string> _nonflags;
  std::string _binary;
  std::string _flag_description;
  std::string _epilogue;
  std::string _preamble;
  size_t _nonflags_required = 0;
  bool _help_enabled = true;
  bool _exit_enabled = true;
  bool _throw_on_validation = false;
  bool _exceptions_enabled = true;
  bool _usage_enabled = true;

  bool CheckIfNonValueFlag(const std::string& flag) const {
    for (auto iter : _required) {
      if (flag == iter.flag || flag == iter.alias)
        return !iter.value;
    }

    for (auto iter : _optional) {
      if (flag == iter.flag || flag == iter.alias)
        return !iter.value;
    }

    return false;
  }

  bool CheckIfValueFlag(const std::string& flag) const {
    for (auto iter : _required) {
      if (flag == iter.flag || flag == iter.alias)
        return iter.value;
    }

    for (auto iter : _optional) {
      if (flag == iter.flag || flag == iter.alias)
        return iter.value;
    }

    return false;
  }

  std::string CheckForValues(const std::vector<Argument>& to_check, const bool required) {
    for (auto iter : to_check) {
      auto arg_iter = _arguments.find(iter.flag);
      if (arg_iter != _arguments.end()) {
        if (iter.value && arg_iter->second.empty()) {
          std::stringstream err;
          err << "Must specify value for " + iter.flag;
          return err.str();
        }

        _arguments[iter.alias] = arg_iter->second;
        continue;
      }

      arg_iter = _arguments.find(iter.alias);
      if (arg_iter != _arguments.end() && !arg_iter->second.empty()) {
        if (iter.value && arg_iter->second.empty()) {
          std::stringstream err;
          err << "Must specify value for " + iter.alias;
          return err.str();
        }

        _arguments[iter.flag] = arg_iter->second;
        continue;
      }

      if (required) {
        std::stringstream err;
        err << "Must specify " + iter.flag;
        if (!iter.alias.empty())
          err << " or " + iter.alias;
        return err.str();
      }
    }
    return "";
  }

  bool TryFlagValueSplit(const std::string& arg, std::pair<std::string, std::string>& flag_value) {
    size_t pos = arg.find_first_of('=');
    std::string current_flag(arg.substr(0, pos));
    const bool has_separator = (pos != std::string::npos);
    const bool is_expected = this->CheckIfValueFlag(current_flag);
    if (has_separator && is_expected) {
      flag_value.first = current_flag;
      flag_value.second = arg.substr(pos + 1);
      return true;
    }
    return false;
  }

  void AddFallbackValues() {
    for (auto iter : _optional) {
      auto flag_iter = _arguments.find(iter.flag);
      if (flag_iter == _arguments.end() && !iter.fallback.empty())
        _arguments[iter.flag] = iter.fallback;

      auto alias_iter = _arguments.find(iter.alias);
      if (alias_iter == _arguments.end() && !iter.fallback.empty())
        _arguments[iter.alias] = iter.fallback;
    }

    for (auto iter : _required) {
      auto flag_iter = _arguments.find(iter.flag);
      if (flag_iter == _arguments.end() && !iter.fallback.empty())
        _arguments[iter.flag] = iter.fallback;

      auto alias_iter = _arguments.find(iter.alias);
      if (alias_iter == _arguments.end() && !iter.fallback.empty())
        _arguments[iter.alias] = iter.fallback;
    }
  }

  std::string Parse(int argc, char* argv[]) {
    _binary = argv[0];
    const int total_flags = _required.size() + _optional.size();
    int flags_encountered = 0;
    bool delim_encountered = false;
    for (int i = 1; i < argc; ++i) {
      // Check if we encountered the non-flag delimiter
      const std::string current(argv[i]);
      if (current == std::string("--")) {
        delim_encountered = true;
        continue;
      }

      // Check for explicit non-flags
      if (delim_encountered) {
        _nonflags.push_back(argv[i]);
        continue;
      }

      if (this->CheckIfNonValueFlag(current)) {
        _arguments[current] = "";
        ++flags_encountered;
        if (flags_encountered >= total_flags)
          delim_encountered = true;
        continue;
      }

      if (this->CheckIfValueFlag(current)) {
        if (i + 1 == argc)
          return "Must set value for " + current;
        _arguments[current] = argv[i + 1];
        i++;
        flags_encountered++;
        if (flags_encountered >= total_flags)
          delim_encountered = true;
        continue;
      }

      std::pair<std::string, std::string> flag_value;
      if (this->TryFlagValueSplit(current, flag_value)) {
        _arguments[flag_value.first] = flag_value.second;
        flags_encountered++;
        if (flags_encountered >= total_flags)
          delim_encountered = true;
        continue;
      }

      // Otherwise set to non-flag
      _nonflags.push_back(argv[i]);
    }

    if (_nonflags.size() != _nonflags_required && _nonflags_required == 0)
      return "Unknown arguments";
    else if (_nonflags.size() != _nonflags_required)
      return "Unknown arguments or user must specify " + std::to_string(_nonflags_required) + " non-flags";

    std::string result = this->CheckForValues(_required, true);
    if (result.empty())
      result = this->CheckForValues(_optional, false);
    return result;
  }

  size_t DetermineNumCharsToWrite(const std::string& description) const {
    size_t location = 49;
    while (std::isalpha(description[location]) && location > 0) --location;
    if (location == 0 && std::isalpha(description[location])) return 50;
    return location + 1;
  }

  std::string FormatDescription(const std::string& description) const {
    if (description.size() <= 50) return description;
    std::stringstream stream;
    stream.width(50);
    size_t count = 0;
    while (count < (description.size() - 50)) {
      size_t to_write = this->DetermineNumCharsToWrite(description.substr(count));

      // Determine where to start writing the next line
      size_t skip = 0;
      while (std::isblank(description[count + skip]) && skip < 50)
        ++skip;

      // Don't write a blank line
      count += skip;
      if (skip == 50)
        continue;

      stream.write(&description[count], to_write);
      stream << "\n                              ";
      count += to_write;
    }

    // Determine where to start writing the next line
    size_t skip = 0;
    while (std::isblank(description[count]) && count < 50) ++skip;
    if (skip != 50) {
      count += skip;
      stream.write(&description[count], int(description.size() - count));
    }
    return stream.str();
  }

  std::string GenerateArgumentUsage(const std::vector<Argument>& arguments) const {
    std::stringstream output;
    for (size_t i = 0; i < arguments.size(); ++i) {
      std::stringstream flag_ids;
      flag_ids << "    " << arguments[i].flag;
      if (arguments[i].value)
        flag_ids << "=value";
      if (!arguments[i].flag.empty() && !arguments[i].alias.empty())
        flag_ids << "/";
      if (!arguments[i].alias.empty()) {
        flag_ids << arguments[i].alias;
        if (arguments[i].value)
          flag_ids << "=value";
      }

      output << std::left << std::setw(30) << flag_ids.str();
      output << std::left << this->FormatDescription(arguments[i].description);
      output << '\n';
    }
    return output.str();
  }

  void GenerateUsage() {
    std::stringstream output;
    if (_required.size() > 0)
      output << "\n  Required flags:\n";
    output << this->GenerateArgumentUsage(_required);

    if (_optional.size() > 0)
      output << "\n  Optional flags:\n";
    output << this->GenerateArgumentUsage(_optional);

    if (_nonflags_required > 0) {
      output << "\n  " << _nonflags_required << " non-flags are required" << std::endl;
    }

    _flag_description = output.str();

    output.str("");
    output << "Usage: " << _binary << ' ';
    for (size_t i = 0; i < _optional.size(); ++i) {
      output << "[" << _optional[i].flag;
      if (!_optional[i].flag.empty()) {
        if (_optional[i].value)
          output << "=value";
      }

      if (!_optional[i].alias.empty()) {
        output << "|" << _optional[i].alias;
        if (_optional[i].value)
          output << "=value";
      }
      output << "] ";
    }

    for (size_t i = 0; i < _required.size(); ++i) {
      output << _required[i].flag;
      if (_required[i].value)
        output << "=value";
      if (!_required[i].alias.empty()) {
          output << "|" << _required[i].alias;
          if (_required[i].value)
            output << "=value";
      }
      output << " ";
    }

    if (_nonflags_required > 0) {
      output << "<--> ";
      for (size_t i = 0; i < _nonflags_required; ++i) {
        output << "nonflag" << i + 1 << " ";
      }
    }

    output << "\n";
    _preamble = output.str();
  }
};

// Parses and verifies the arguments to ensure the flags are recognized and well-formed
#define SARGS_INITIALIZE(argc, argv) \
  sargs::Args::Default().Initialize(argc, argv)

// Tells Sargs that a flag is required and will have no value
#define SARGS_REQUIRED_FLAG(flag, alias, description) \
  sargs::Args::Default().AddRequiredFlag(flag, alias, description)

// Tells Sargs that a flag is required and will have a value with no default value
#define SARGS_REQUIRED_FLAG_VALUE(flag, alias, description) \
  sargs::Args::Default().AddRequiredFlagValue(flag, alias, description, "")

// Tells Sargs that a flag is required and will have a value with a default value
#define SARGS_REQUIRED_FLAG_VALUE_DEFAULT(flag, alias, description, fallback) \
  sargs::Args::Default().AddRequiredFlagValue(flag, alias, description, fallback)

// Tells Sargs that an optional flag should be expected with no value
#define SARGS_OPTIONAL_FLAG(flag, alias, description) \
  sargs::Args::Default().AddOptionalFlag(flag, alias, description)

// Tells Sargs that an optional flag should be expected and will have a value with no default value
#define SARGS_OPTIONAL_FLAG_VALUE(flag, alias, description) \
  sargs::Args::Default().AddOptionalFlagValue(flag, alias, description, "")

// Tells Sargs that an optional flag should be expected, will have a value and a default value
#define SARGS_OPTIONAL_FLAG_VALUE_DEFAULT(flag, alias, description, fallback) \
  sargs::Args::Default().AddOptionalFlagValue(flag, alias, description, fallback)

// Replace the default preamble with a custom one
#define SARGS_SET_PREAMBLE(preamble) \
  sargs::Args::Default().SetPreamble(preamble)

// Set the epilogue
#define SARGS_SET_EPILOGUE(epilogue) \
  sargs::Args::Default().SetEpilogue(epilogue)

// Replace the default flag description string with a custom one
#define SARGS_SET_FLAG_DESCRIPTION(flag_description) \
  sargs::Args::Default().SetFlagDescription(flag_description)

// Gets the currently set preamble string
#define SARGS_GET_PREAMBLE() \
  sargs::Args::Default().GetPreamble()

// Gets the currently set epilogue string
#define SARGS_GET_EPILOGUE() \
  sargs::Args::Default().GetEpilogue()

// Gets the currently set flag description string
#define SARGS_GET_FLAG_DESCRIPTION() \
  sargs::Args::Default().GetFlagDescription()

// Get the binary string which is passed as argv[0]
#define SARGS_GET_BINARY() \
  sargs::Args::Default().Binary()

// Print the usage to the specified stream
#define SARGS_PRINT_USAGE(ostream) \
  sargs::Args::Default().PrintUsage(ostream)

// Print the usage to std::cout
#define SARGS_PRINT_USAGE_TO_COUT() \
  sargs::Args::Default().PrintUsage(std::cout)

// Require that at least count non-flags are specified by the user
#define SARGS_REQUIRE_NONFLAGS(count) \
  sargs::Args::Default().RequireNonFlags(count)

// Get a non-flag based on the index it was specified by the user
#define SARGS_GET_NONFLAG(index) \
  sargs::Args::Default().GetNonFlag(index)

// Gets all non-flags
#define SARGS_GET_NONFLAGS() \
  sargs::Args::Default().GetNonFlags()

// Get the value of a flag as an int64_t
#define SARGS_GET_INT64(flag) \
  sargs::Args::Default().GetAsInt64(flag)

// Get the value of a flag as an int32_t
#define SARGS_GET_INT32(flag) \
  sargs::Args::Default().GetAsInt32(flag)

// Get the value of a flag as an int8_t
#define SARGS_GET_INT8(flag) \
  sargs::Args::Default().GetAsInt8(flag)

// Get the value of a flag as a std::string
#define SARGS_GET_STRING(flag) \
  sargs::Args::Default().GetAsString(flag)

// Get the value of a flag as a float
#define SARGS_GET_FLOAT(flag) \
  sargs::Args::Default().GetAsFloat(flag)

// Return a bool of the flag was specified
#define SARGS_HAS(flag) \
  sargs::Args::Default().Has(flag)

// Disable default -h and --help flags. These will do nothing if specified by
// the user when this is called before SARGS_INITIALIZE()
#define SARGS_DISABLE_HELP() \
  sargs::Args::Default().DisableHelp()

// Disables the call to std::exit if there is a problem during initialization
#define SARGS_DISABLE_EXIT() \
  sargs::Args::Default().DisableExit()

// Disables all exceptions in Sargs
#define SARGS_DISABLE_EXCEPTIONS() \
  sargs::Args::Default().DisableExceptions()

// Disables printing usage if an error is encountered during initialization
#define SARGS_DISABLE_USAGE() \
  sargs::Args::Default().DisableUsage()

}  // namespace sargs
