#pragma once

#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <iomanip>
#include "flag_parser.h"

namespace sarg {

struct Argument {
	Argument(const std::string& _flag,
					 const std::string& _alias,
					 const std::string& _description) :
		flag(_flag), alias(_alias), description(_description) {}

  std::string flag;
  std::string alias;
  std::string description;
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
    this->GenerateUsage();
    _parser.Parse(argc, argv);
    _arguments = _parser.GetFlags();
    this->Validate();
  }

  std::string GetAsString(const std::string& flag) const {
    if (flag.empty()) { throw std::runtime_error("Flag query empty"); }

    return _parser.GetValue(flag);
  }

  float GetAsFloat(const std::string& flag) const {
    if (flag.empty()) { throw std::runtime_error("Flag query empty"); }

    const std::string value_str(_parser.GetValue(flag));
    const float value = std::strtof(value_str.c_str(), nullptr);
    if (errno == ERANGE) {
      throw std::runtime_error("Could not convert " +  value_str +
                   " to float");
    }

    return value;
  }

  int64_t GetAsInt64(const std::string& flag) const {
    if (flag.empty()) { throw std::runtime_error("Flag query empty"); }

    const std::string value_str(_parser.GetValue(flag));
    const int64_t value = std::strtol(value_str.c_str(), nullptr, 0);
    const bool range_error = (errno == ERANGE);
    if (range_error || (value == 0 && flag != "0")) {
      throw std::runtime_error("Could not convert " +  value_str +
                   " to int64_t");
    }

    return value;
  }

  bool Has(const std::string& flag) const {
  	return _parser.Has(flag);
  }

  std::string GetNonFlag(const size_t index) const {
    return _parser.GetNonFlag(index);
  }

  void AddRequiredFlag(const std::string& flag, const std::string& alias,
                       const std::string& description) {
    _required.emplace_back(flag, alias, description);
  }

  void AddOptionalFlag(const std::string& flag, const std::string& alias,
                       const std::string& description) {
    _optional.emplace_back(flag, alias, description);
  }

  void RequireNonFlags(const int count) {
  	_nonflags_required = count;
  }

  void PrintUsage(std::ostream& output) {
  	output << _preamble << _usage << _epilouge;
  }

  void SetUsagePreamble(const std::string& preamble) {
  	_preamble = preamble;
  }

  void SetUsageEpilouge(const std::string& epilouge) {
  	_epilouge = epilouge;
  }

  void SetUsage(const std::string& usage) {
  	_usage = usage;
  }

 private:
  FlagParser _parser;
  std::vector<Argument> _required;
  std::vector<Argument> _optional;
  std::map<std::string, std::string> _arguments;
  std::string _usage;
  std::string _epilouge;
  std::string _preamble;
  int _nonflags_required = 0;

  void Validate() {
  	for (size_t i = 0; i < _required.size(); ++i) {
  		const std::string flag = _required[i].flag;
  		auto iter = _arguments.find(flag);
  		if (iter == _arguments.end()) {
  			std::string alias = _required[i].alias;
  			if (alias.empty())
  				throw std::runtime_error(flag + " required, but not specified");

  			iter = _arguments.find(alias);
  			if (iter == _arguments.end()) {
  				throw std::runtime_error(flag + " or " + alias +
  																 " required, but not specified");
  			}

  			// Add mapping for flag to alia's result
  			_arguments[flag] = _arguments[alias];
  		} else if (!_required[i].alias.empty()) {
	  		const std::string alias = _required[i].alias;
  			auto alias_iter = _arguments.find(alias);
  			if (alias_iter != _arguments.end()) {
  				throw std::runtime_error("Cannot specify " + flag + " and " + alias +
  																 ", they mean the same thing");
  			}

  			// Add a mapping for alias to flag's result
  			_arguments[alias] = iter->second;
  		}
  	}

  	if (_parser.GetNonFlags().size() < _nonflags_required) {
  		throw std::runtime_error("Must specify at least " +
  			std::to_string(_nonflags_required) + " non-flags");
  	}
  }

  std::string FormatDescription(const std::string& description) {
  	if (description.size() <= 50) return description;
  	std::stringstream stream;
  	stream.width(50);
  	size_t count = 0;
  	while (count < description.size()) {
  		stream.write(&description[count], std::min(50, int(description.size()) - 50));
  		stream << "\n                              ";
  		count += 50;
  	}

  	return stream.str();
  }

  void GenerateUsage() {
  	std::stringstream output;
  	if (_required.size() > 0)
  		output << "  Required flags:\n";

  	for (size_t i = 0; i < _required.size(); ++i) {
  		std::stringstream flag_ids;
  		flag_ids << "    " << _required[i].flag;
  		if (!_required[i].flag.empty() && !_required[i].alias.empty())
  			flag_ids << "/";
  		flag_ids << _required[i].alias;

  		output << std::left << std::setw(30) << flag_ids.str();
  		output << std::left << this->FormatDescription(_required[i].description);
  		output << '\n';
  	}

  	if (_optional.size() > 0)
  		output << "  Optional flags:\n";
  	for (size_t i = 0; i < _optional.size(); ++i) {
  		std::stringstream flag_ids;
  		flag_ids << "    " << _optional[i].flag;
  		if (!_optional[i].flag.empty() && !_optional[i].alias.empty())
  			flag_ids << "/";
  		flag_ids << _optional[i].alias;

  		output << std::setw(36) << flag_ids.str();
  		output << std::setw(40) << std::left << _optional[i].description;
  	}

  	if (_nonflags_required > 0) {
  		output << "  " << _nonflags_required << " non-flags are required"
  					 << std::endl;
  	}

  	_usage = output.str();
  }
};

#define SARG_INITIALIZE(argc, argv) \
  Args::Default().Initialize(argc, argv)

#define SARG_REQUIRED_FLAG(flag, alias, description) \
  Args::Default().AddRequiredFlag(flag, alias, description)

#define SARG_OPTIONAL_FLAG(flag, alias, description) \
  Args::Default().AddOptionalFlag(flag, alias, description)

#define SARG_SET_USAGE_PREAMBLE(preamble) \
  Args::Default().SetUsagePreamble(preamble)

#define SARG_SET_USAGE_EPILOGUE(epilouge) \
  Args::Default().SetUsageEpilouge(epilouge)

#define SARG_SET_USAGE(usage) \
  Args::Default().SetUsage(usage)

#define SARG_PRINT_USAGE(ostream) \
  Args::Default().PrintUsage(ostream)

#define SARG_PRINT_USAGE_TO_COUT() \
  Args::Default().PrintUsage(std::cout)

#define SARG_REQUIRE_NONFLAGS(count) \
  Args::Default().RequireNonFlags(count)

#define SARG_GET_NONFLAG(index) \
  Args::Default().GetNonFlag(index)

#define SARG_GET_INT64(flag) \
  Args::Default().GetAsInt64(flag)

#define SARG_GET_STRING(flag) \
  Args::Default().GetAsString(flag)

#define SARG_GET_FLOAT(flag) \
  Args::Default().GetAsFloat(flag)

#define SARG_HAS(flag) \
  Args::Default().Has(flag)

}  // namespace sarg
