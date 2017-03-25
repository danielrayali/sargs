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

  bool GetAsString(const std::string& flag, std::string& value) const {
		if (flag.empty()) { throw std::runtime_error("Flag query empty"); }

		auto iter = _arguments.find(flag);
		if (iter == _arguments.end())
			return false;
		value = iter->second;
		return true;
  }

  std::string GetAsString(const std::string& flag) const {
    std::string value;
    if (!this->GetAsString(flag, value))
    	throw std::runtime_error(flag + " was not specified");
    return value;
  }

  bool GetAsFloat(const std::string& flag, float& value) const {
    if (flag.empty()) { throw std::runtime_error("Flag query empty"); }

    auto iter = _arguments.find(flag);
    if (iter == _arguments.end())
    	return false;

    const std::string value_str(iter->second);
    const float myvalue = std::strtof(value_str.c_str(), nullptr);
    if (errno == ERANGE)
      throw std::runtime_error("Could not convert " +  value_str + " to float");

		value = myvalue;
		return true;
  }

  float GetAsFloat(const std::string& flag) const {
  	float value;
  	if (!this->GetAsFloat(flag, value))
    	throw std::runtime_error(flag + " was not specified");
  	return value;
  }

  bool GetAsInt64(const std::string& flag, int64_t& value) const {
    if (flag.empty()) { throw std::runtime_error("Flag query empty"); }

    auto iter = _arguments.find(flag);
    if (iter == _arguments.end())
    	return false;

    const std::string value_str(iter->second);
    const int64_t myvalue = std::strtol(value_str.c_str(), nullptr, 0);
    const bool range_error = (errno == ERANGE);
    if (range_error || (myvalue == 0 && flag != "0")) {
      throw std::runtime_error("Could not convert " +  value_str +
                   " to int64_t");
    }

    value = myvalue;
    return true;
  }

  int64_t GetAsInt64(const std::string& flag) const {
  	int64_t value;
  	if (!this->GetAsInt64(flag, value))
    	throw std::runtime_error(flag + " was not specified");
  	return value;
  }

  bool Has(const std::string& flag) const {
  	return _arguments.find(flag) != _arguments.end();
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

  void SetPreamble(const std::string& preamble) {
  	_preamble = preamble;
  }

  void SetEpilouge(const std::string& epilouge) {
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
  	std::vector<std::pair<std::string, std::string>> to_add;
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
  			to_add.push_back(std::make_pair(flag, _arguments[alias]));
  		} else if (!_required[i].alias.empty()) {
	  		const std::string alias = _required[i].alias;
  			auto alias_iter = _arguments.find(alias);
  			if (alias_iter != _arguments.end()) {
  				throw std::runtime_error("Cannot specify " + flag + " and " + alias +
  																 ", they mean the same thing");
  			}

  			// Add a mapping for alias to flag's result
  			to_add.push_back(std::make_pair(alias, iter->second));
  		}
  	}

  	if (_parser.GetNonFlags().size() < _nonflags_required) {
  		throw std::runtime_error("Must specify at least " +
  			std::to_string(_nonflags_required) + " non-flags");
  	}

  	for (auto& iter : to_add)
  		_arguments[iter.first] = iter.second;
  }

  std::string FormatDescription(const std::string& description) const {
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

  std::string GenerateArgumentUsage(const std::vector<Argument>& arguments) const {
  	std::stringstream output;
  	for (size_t i = 0; i < arguments.size(); ++i) {
  		std::stringstream flag_ids;
  		flag_ids << "    " << arguments[i].flag;
  		if (!arguments[i].flag.empty() && !arguments[i].alias.empty())
  			flag_ids << "/";
  		flag_ids << arguments[i].alias;

  		output << std::left << std::setw(30) << flag_ids.str();
  		output << std::left << this->FormatDescription(arguments[i].description);
  		output << '\n';
  	}
  	return output.str();
  }

  void GenerateUsage() {
  	std::stringstream output;
  	if (_required.size() > 0)
  		output << "  Required flags:\n";
  	output << this->GenerateArgumentUsage(_required);

  	if (_optional.size() > 0)
  		output << "\n  Optional flags:\n";
  	output << this->GenerateArgumentUsage(_optional);

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
  Args::Default().SetPreamble(preamble)

#define SARG_SET_USAGE_EPILOGUE(epilouge) \
  Args::Default().SetEpilouge(epilouge)

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
