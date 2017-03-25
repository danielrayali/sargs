#pragma once

#include <cerrno>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace sarg {

class FlagParser {
 public:
	FlagParser() = default;

	FlagParser(int argc, char* argv[]) { this->Parse(argc, argv); }

	~FlagParser() = default;

	void Parse(int argc, char* argv[]) {
		_binary = argv[0];
		bool parse_as_flag = false;
		for (int i = 1; i < argc; ++i) {
			const std::string current(argv[i]);
			if (current == "--") {
				parse_as_flag = true;
				continue;
			}

			const bool is_flag = this->IsFlag(argv[i]);
			if (parse_as_flag || !is_flag) {
				// Parse as non-flag
				_nonflags.push_back(argv[i]);
				parse_as_flag = true;
				continue;
			}

			size_t pos = current.find_first_of('=');
			if (pos != std::string::npos) {
				_arguments[current.substr(0, pos)] = argv[i] + pos + 1;
			} else if (((i + 1) != argc) && !this->IsFlag(argv[i + 1])) {
				_arguments[current] = argv[i + 1];
				i++;
				continue;
			} else {
				_arguments[current] = nullptr;
			}
		}
	}

	bool Has(const std::string& flag) const {
		if (flag.empty()) { throw std::runtime_error("Flag unspecified"); }
		return (_arguments.find(flag) != _arguments.end());
	}

	std::string GetNonFlag(const size_t index) const {
		if (index > _nonflags.size()) {
			throw std::runtime_error("Nonflag at index " + std::to_string(index) +
				" not specified");
		}
		return _nonflags.at(index);
	}

	std::vector<std::string> GetNonFlags() const {
		std::vector<std::string> nonflags;
		for (int i = 0; i < _nonflags.size(); ++i)
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
		if (flag.empty()) { throw std::runtime_error("Flag unspecified"); }

		auto iter = _arguments.find(flag);
		if (iter == _arguments.end())
			throw std::runtime_error(flag + " required but not specified");

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

 private:
	std::map<std::string, char*> _arguments;
	std::vector<char*> _nonflags;
	std::string _binary;

	bool IsFlag(char* arg) {
		if (arg[0] == '-') return true;
		return false;
	}
};

struct Argument {
	Argument(const std::string& _flag,
					 const std::string& _alias,
					 const std::string& _description,
					 const bool _value = false) :
		flag(_flag), alias(_alias), description(_description), value(_value) {}

  std::string flag;
  std::string alias;
  std::string description;
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
    _arguments = _parser.GetFlags();
    this->GenerateUsage();

    auto iter = _arguments.find("--help");
  	bool usage = (_help_enabled && iter != _arguments.end());
  	try { this->Validate(); }
  	catch (...) { usage = true; }

    if (usage) {
    	this->PrintUsage(std::cout);
    	exit(0);
    }
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
                       const std::string& description, bool value = false) {
    _required.emplace_back(flag, alias, description, value);
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

  void DisableHelp() {
  	_help_enabled = false;
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
  bool _help_enabled = true;

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
  		if (!arguments[i].flag.empty() && !arguments[i].alias.empty())
  			flag_ids << "/";
  		flag_ids << arguments[i].alias;

  		output << std::left << std::setw(30) << flag_ids.str();
  		output << std::left << this->FormatDescription(arguments[i].description);
  		output << '\n';
  	}
  	return output.str();
  }

  void GenerateFlagUsage() {
  	std::stringstream output;
  	if (_required.size() > 0)
  		output << "  Required flags:\n";
  	output << this->GenerateArgumentUsage(_required);

  	if (_optional.size() > 0)
  		output << "\n  Optional flags:\n";
  	output << this->GenerateArgumentUsage(_optional);

  	if (_nonflags_required > 0) {
  		output << "\n  " << _nonflags_required << " non-flags are required"
  					 << std::endl;
  	}

  	_usage = output.str();
  }

  void GenerateUsage() {
  	std::stringstream output;
  	output << "Usage: " << _parser.GetBinary() << ' ';
  	if (_optional.size() > 0) {
	  	output << "<";
	  	for (size_t i = 0; i < _optional.size() - 1; ++i) {
	  		output << _optional[i].flag << '|';
	  	}
	  	output << _optional[_optional.size() - 1].flag << "> ";
	  }

	  for (size_t i = 0; i < _required.size(); ++i) {
	  	output << _required[i].flag;
	  	if (_required[i].value)
	  		output << "=value";
	  	output << " ";
	  }

	  if (_nonflags_required > 0) {
	  	output << "-- ";
	  	for (size_t i = 0; i < _nonflags_required; ++i) {
	  		output << "nonflag" << i + 1 << " ";
	  	}
	  }

	  output << "\n\n";
	  _preamble = output.str();
  }
};

#define SARG_INITIALIZE(argc, argv) \
  sarg::Args::Default().Initialize(argc, argv)

#define SARG_REQUIRED_FLAG(flag, alias, description) \
  sarg::Args::Default().AddRequiredFlag(flag, alias, description)

#define SARG_REQUIRED_FLAG_VALUE(flag, alias, description) \
  sarg::Args::Default().AddRequiredFlag(flag, alias, description, true)

#define SARG_OPTIONAL_FLAG(flag, alias, description) \
  sarg::Args::Default().AddOptionalFlag(flag, alias, description)

#define SARG_SET_USAGE_PREAMBLE(preamble) \
  sarg::Args::Default().SetPreamble(preamble)

#define SARG_SET_USAGE_EPILOGUE(epilouge) \
  sarg::Args::Default().SetEpilouge(epilouge)

#define SARG_SET_USAGE(usage) \
  sarg::Args::Default().SetUsage(usage)

#define SARG_PRINT_USAGE(ostream) \
  sarg::Args::Default().PrintUsage(ostream)

#define SARG_PRINT_USAGE_TO_COUT() \
  sarg::Args::Default().PrintUsage(std::cout)

#define SARG_REQUIRE_NONFLAGS(count) \
  sarg::Args::Default().RequireNonFlags(count)

#define SARG_GET_NONFLAG(index) \
  sarg::Args::Default().GetNonFlag(index)

#define SARG_GET_INT64(flag) \
  sarg::Args::Default().GetAsInt64(flag)

#define SARG_GET_STRING(flag) \
  sarg::Args::Default().GetAsString(flag)

#define SARG_GET_FLOAT(flag) \
  sarg::Args::Default().GetAsFloat(flag)

#define SARG_HAS(flag) \
  sarg::Args::Default().Has(flag)

#define SARG_DISABLE_HELP() \
  sarg::Args::Default().DisableHelp()

}  // namespace sarg
