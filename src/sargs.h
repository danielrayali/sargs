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

class SargsUsage : public std::invalid_argument {
 public:
  SargsUsage(const std::string& message) : std::invalid_argument(message) {};
  ~SargsUsage() = default;
};

class FlagParser {
 public:
  FlagParser() = default;

  FlagParser(int argc, char* argv[]) { this->Parse(argc, argv); }

  ~FlagParser() = default;

  void Parse(int argc, char* argv[]) {
    _binary = argv[0];
    bool delim_encountered = false;
    for (int i = 1; i < argc; ++i) {
      // Check for explicit non-flags
      if (delim_encountered) {
        _nonflags.push_back(argv[i]);
        continue;
      }

      // Check if we encountered the non-flag delimiter
      if (std::string(argv[i]) == std::string("--")) {
        delim_encountered = true;
        continue;
      }

      // Check for exact matches of a flag the user requested
      const std::string current(argv[i]);
      if (this->Contains(_expected_flags, current)) {
        _arguments[current] = nullptr;
        continue;
      }

      // Check if this is a stand alone value flag
      if (this->Contains(_expected_value_flags, current)) {
        _arguments[current] = argv[i + 1];
        i++;
        continue;
      }

      // Check for flags that set a value through an equals (e.g. --foo=bar)
      size_t pos = current.find_first_of('=');
      std::string current_flag(current.substr(0, pos));
      const bool has_separator = (pos != std::string::npos);
      const bool is_expected = this->Contains(_expected_value_flags, current_flag);
      if (has_separator && is_expected) {
        _arguments[current_flag] = argv[i] + pos + 1;
        continue;
      }

      // If all else fails, set this as a non-flag
      _nonflags.push_back(argv[i]);
    }
  }

  bool Has(const std::string& flag) const {
    if (flag.empty()) { throw SargsError("Flag unspecified"); }
    return (_arguments.find(flag) != _arguments.end());
  }

  std::string GetNonFlag(const size_t index) const {
    if (index > _nonflags.size()) {
      throw SargsError("Nonflag at index " + std::to_string(index) +
        " not specified");
    }
    return _nonflags.at(index);
  }

  std::vector<std::string> GetNonFlags() const {
    std::vector<std::string> nonflags;
    for (size_t i = 0; i < _nonflags.size(); ++i)
      nonflags.push_back(std::string(_nonflags[i]));
    return nonflags;
  }

  std::map<std::string, std::string> GetFlags() const {
    std::map<std::string, std::string> result;
    for (auto& iter : _arguments)
      if (iter.second != nullptr)
        result[iter.first] = std::string(iter.second);
      else
        result[iter.first] = "";

    return result;
  }

  std::string GetValue(const std::string& flag) const {
    if (flag.empty()) { throw SargsError("Flag unspecified"); }

    auto iter = _arguments.find(flag);
    if (iter == _arguments.end())
      return {};

    return std::string(iter->second);
  }

  void Print() const {
    printf("Flags\n");
    for (auto pair_iter : _arguments) {
      printf(" %s = %s\n", pair_iter.first.c_str(), pair_iter.second);
    }

    printf("\nNonflags\n");
    for (auto iter : _nonflags) {
      printf(" %s\n", iter);
    }
  }

  std::string GetBinary() const {
    return _binary;
  }

  void AddExpectedFlag(const std::string& flags) {
    _expected_flags.push_back(flags);
  }

  void AddExpectedValueFlag(const std::string& value_flags) {
    _expected_value_flags.push_back(value_flags);
  }

 private:
  std::vector<std::string> _expected_flags;
  std::vector<std::string> _expected_value_flags;
  std::map<std::string, char*> _arguments;
  std::vector<char*> _nonflags;
  std::string _binary;

  bool IsFlag(char* arg) {
    if (arg[0] == '-') return true;
    return false;
  }

  bool Contains(const std::vector<std::string>& to_search, const std::string& what) {
    return (std::find(std::begin(to_search), std::end(to_search), what) != std::end(to_search));
  }
};

struct Argument {
  Argument(const std::string& _flag,
           const std::string& _alias,
           const std::string& _description,
           const bool _value = false) :
    flag(_flag), alias(_alias), description(_description), value(_value) {}

  Argument(const std::string& _flag,
           const std::string& _alias,
           const std::string& _description,
           const std::string& _fallback,
           const bool _value = false) :
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

    this->GenerateFlagUsage();
    _parser.Parse(argc, argv);
    for (auto iter : _parser.GetFlags())
      _arguments[iter.first] = iter.second;
    this->GenerateUsage();

    const bool help_specified = _parser.Has("--help") || _parser.Has("-h");
    bool usage = (_help_enabled && help_specified);
    try { this->Validate(); }
    catch (...) {
      if (_throw_on_validation && !usage) throw;
      usage = true;
    }

    if (usage) {
      this->PrintUsage(std::cout);
      std::stringstream stream;
      this->PrintUsage(stream);
      throw SargsUsage(stream.str());
    }
  }

  bool GetAsString(const std::string& flag, std::string& value) const {
    if (flag.empty()) { throw SargsError("Flag query empty"); }

    auto iter = _arguments.find(flag);
    if (iter == _arguments.end())
      return false;
    value = iter->second;
    return true;
  }

  std::string GetAsString(const std::string& flag) const {
    std::string value;
    if (!this->GetAsString(flag, value))
      throw SargsError(flag + " was not specified");
    return value;
  }

  bool GetAsFloat(const std::string& flag, float& value) const {
    if (flag.empty()) { throw SargsError("Flag query empty"); }

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
      throw SargsError(flag + " was not specified");
    return value;
  }

  bool GetAsInt64(const std::string& flag, int64_t& value) const {
    if (flag.empty()) { throw SargsError("Flag query empty"); }

    auto iter = _arguments.find(flag);
    if (iter == _arguments.end())
      return false;

    const std::string value_str(iter->second);
    const int64_t myvalue = std::strtol(value_str.c_str(), nullptr, 0);
    const bool range_error = (errno == ERANGE);
    if (range_error || (myvalue == 0 && flag != "0")) {
      throw SargsError("Could not convert " +  value_str +
        " to int64_t");
    }

    value = myvalue;
    return true;
  }

  int64_t GetAsInt64(const std::string& flag) const {
    int64_t value;
    if (!this->GetAsInt64(flag, value))
      throw SargsError(flag + " was not specified");
    return value;
  }

  bool Has(const std::string& flag) const {
    return _arguments.find(flag) != _arguments.end();
  }

  std::string GetNonFlag(const size_t index) const {
    return _parser.GetNonFlag(index);
  }

  void AddRequiredFlag(const std::string& flag, const std::string& alias,
                       const std::string& description, bool value = false) {
    _required.emplace_back(flag, alias, description, value);
    this->AddExpectedFlagToParser(flag, value);
    this->AddExpectedFlagToParser(alias, value);
  }

  void AddOptionalFlag(const std::string& flag, const std::string& alias,
                       const std::string& description, bool value = false) {
    _optional.emplace_back(flag, alias, description, value);
    this->AddExpectedFlagToParser(flag, value);
    this->AddExpectedFlagToParser(alias, value);
  }

  void AddOptionalFlag(const std::string& flag, const std::string& alias,
                       const std::string& description, const std::string& fallback,
                       bool value = false) {
    _optional.emplace_back(flag, alias, description, fallback, value);
    if (!flag.empty())
      _arguments[flag] = fallback;
    if (!alias.empty())
      _arguments[alias] = fallback;
    this->AddExpectedFlagToParser(flag, value);
    this->AddExpectedFlagToParser(alias, value);
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

  void ThrowOnValidation() {
    _throw_on_validation = true;
  }

 private:
  FlagParser _parser;
  std::vector<Argument> _required;
  std::vector<Argument> _optional;
  std::map<std::string, std::string> _arguments;
  std::string _flag_description;
  std::string _epilogue;
  std::string _preamble;
  size_t _nonflags_required = 0;
  bool _help_enabled = true;
  bool _throw_on_validation = false;

  void AddExpectedFlagToParser(const std::string& flag, const bool value) {
    if (flag.empty())
        return;

    if (value)
      _parser.AddExpectedValueFlag(flag);
    else
      _parser.AddExpectedFlag(flag);
  }

  void AddArgument(Argument& iter) {
      if (!_parser.Has(iter.flag) && !_parser.Has(iter.alias))
        throw SargsUsage("Must specify " + iter.flag + " or " + iter.alias);
      std::string value;
      if (iter.value) {
        std::string flag_value = _parser.GetValue(iter.flag);
        std::string alias_value = _parser.GetValue(iter.alias);
        if (flag_value.empty() && alias_value.empty())
          throw SargsUsage("Must specify a value for " + iter.flag + " or " + iter.alias);
        value = flag_value.empty() ? alias_value : flag_value;
      }
      _arguments[iter.flag] = value;
      _arguments[iter.alias] = value;
  }

  void Validate() {
    for (auto& iter : _required)
      this->AddArgument(iter);

    for (auto iter : _optional)
      this->AddArgument(iter);

    // std::vector<std::pair<std::string, std::string>> to_add;
    // for (size_t i = 0; i < _required.size(); ++i) {
    //   const std::string flag = _required[i].flag;
    //   auto iter = _arguments.find(flag);
    //   if (iter == _arguments.end()) {
    //     std::string alias = _required[i].alias;
    //     if (alias.empty())
    //       throw SargsError(flag + " required, but not specified");

    //     iter = _arguments.find(alias);
    //     if (iter == _arguments.end())
    //       throw SargsError(flag + " or " + alias + " required, but not specified");

    //     if (_required[i].value && iter->second.empty())
    //       throw SargsError("No value specified for " + alias);

    //     // Add mapping for flag to alias's result
    //     to_add.push_back(std::make_pair(flag, _arguments[alias]));
    //   } else if (!_required[i].alias.empty()) {
    //     const std::string alias = _required[i].alias;
    //     auto alias_iter = _arguments.find(alias);
    //     if (alias_iter != _arguments.end())
    //       throw SargsError("Cannot specify " + flag + " and " + alias + ", they mean the same thing");

    //     if (_required[i].value && iter->second.empty())
    //       throw SargsError("No value specified for " + flag);

    //     // Add a mapping for alias to flag's result
    //     to_add.push_back(std::make_pair(alias, iter->second));
    //   }
    // }

    // for (size_t i = 0; i < _optional.size(); ++i) {
    //   const std::string flag = _optional[i].flag;
    //   auto iter = _arguments.find(flag);
    //   if (iter == _arguments.end()) {
    //     std::string alias = _optional[i].alias;
    //     if (alias.empty())
    //       continue;

    //     iter = _arguments.find(alias);
    //     if (iter == _arguments.end()) {
    //       continue;
    //     }

    //     if (_optional[i].value && iter->second.empty())
    //       throw SargsError("No value specified for " + alias);

    //     // Add mapping for flag to alia's result
    //     to_add.push_back(std::make_pair(flag, iter->second));
    //   } else if (!_optional[i].alias.empty()) {
    //     const std::string alias = _optional[i].alias;
    //     auto alias_iter = _arguments.find(alias);
    //     if (alias_iter != _arguments.end()) {
    //       throw SargsError("Cannot specify " + flag + " and " + alias +
    //                                ", they mean the same thing");
    //     }

    //     if (_optional[i].value && iter->second.empty())
    //       throw SargsError("No value specified for " + flag);

    //     // Add a mapping for alias to flag's result
    //     to_add.push_back(std::make_pair(alias, iter->second));
    //   }
    // }

    if (_parser.GetNonFlags().size() < _nonflags_required) {
      throw SargsError("Must specify at least " +
        std::to_string(_nonflags_required) + " non-flags");
    }

    if (_parser.GetNonFlags().size() > _nonflags_required) {
      throw SargsError("Unrecognized flags used");
    }

    // for (auto& iter : to_add)
    //   _arguments[iter.first] = iter.second;
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
      flag_ids << arguments[i].alias;
      if (arguments[i].value)
        flag_ids << "=value";

      output << std::left << std::setw(30) << flag_ids.str();
      output << std::left << this->FormatDescription(arguments[i].description);
      output << '\n';
    }
    return output.str();
  }

  void GenerateFlagUsage() {
    std::stringstream output;
    if (_required.size() > 0)
      output << "\n  Required flags:\n";
    output << this->GenerateArgumentUsage(_required);

    if (_optional.size() > 0)
      output << "\n  Optional flags:\n";
    output << this->GenerateArgumentUsage(_optional);

    if (_nonflags_required > 0) {
      output << "\n  " << _nonflags_required << " non-flags are required"
             << std::endl;
    }

    _flag_description = output.str();
  }

  void GenerateUsage() {
    std::stringstream output;
    output << "Usage: " << _parser.GetBinary() << ' ';
    if (_optional.size() > 0) {
      for (size_t i = 0; i < _optional.size(); ++i) {
        output << "<" << _optional[i].flag;
        if (!_optional[i].flag.empty()) {
          if (_optional[i].value)
            output << "=value";
          if (!_optional[i].flag.empty())
            output << "|";
        }

        if (!_optional[i].alias.empty()) {
          output << _optional[i].alias;
          if (_optional[i].value)
            output << "=value";
        }
        output << "> ";
      }
    }

    for (size_t i = 0; i < _required.size(); ++i) {
      output << _required[i].flag;
      if (_required[i].value)
        output << "=value";
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

// Initializes and validates the configuration for the arguments provided
// No return value. Can throw.
#define SARGS_INITIALIZE(argc, argv) \
  sargs::Args::Default().Initialize(argc, argv)

// Tell Sargs that a flag is required and requires no value to be specified
// Both flag and alias will be available for lookup once Sargs is initialized
// even if the user only specified one. description is used just for usage
// information. If a value is specified it is stored but ignored.
#define SARGS_REQUIRED_FLAG(flag, alias, description) \
  sargs::Args::Default().AddRequiredFlag(flag, alias, description)

// Tell sargs that a flag is required and does require a value. Same behavior
// as SARGS_REQUIRED_FLAG
#define SARGS_REQUIRED_FLAG_VALUE(flag, alias, description) \
  sargs::Args::Default().AddRequiredFlag(flag, alias, description, true)

// Tell sargs that there might be an optional flag that may not have value.
#define SARGS_OPTIONAL_FLAG(flag, alias, description) \
  sargs::Args::Default().AddOptionalFlag(flag, alias, description)

// Tell sargs that there might be an optional flag but it must have a value
#define SARGS_OPTIONAL_FLAG_VALUE(flag, alias, description) \
  sargs::Args::Default().AddOptionalFlag(flag, alias, description, true)

// Tell sargs that there might be an optional flag but it must have a value
#define SARGS_OPTIONAL_FLAG_VALUE_DEFAULT(flag, alias, description, fallback) \
  sargs::Args::Default().AddOptionalFlag(flag, alias, description, fallback, true)

// Replace the default preamble with a custom one
#define SARGS_SET_PREAMBLE(preamble) \
  sargs::Args::Default().SetPreamble(preamble)

// Set the epilogue
#define SARGS_SET_EPILOGUE(epilogue) \
  sargs::Args::Default().SetEpilogue(epilogue)

// Replace the default flag description string with a custom one
#define SARGS_SET_FLAG_DESCRIPTION(flag_description) \
  sargs::Args::Default().SetFlagDescription(flag_description)

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

// Get the value of a flag as an int64_t
#define SARGS_GET_INT64(flag) \
  sargs::Args::Default().GetAsInt64(flag)

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

// Allow the argument validation to throw SargsErrors on the problems
// it finds. Description strings will be provided in the exceptions thrown.
#define SARGS_VALIDATION_THROWS() \
  sargs::Args::Default().ThrowOnValidation()

}  // namespace sargs
