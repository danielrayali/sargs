#pragma once

#include <string>
#include <stdexcept>
#include <map>
#include <list>
#include <cerrno>

namespace sarg {

class FlagParser {
 public:
	FlagParser() = default;

	FlagParser(int argc, char* argv[]) { this->Parse(argc, argv); }

	~FlagParser() = default;

	void Parse(int argc, char* argv[]) {
		_program = argv[0];
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

 private:
	std::map<std::string, char*> _arguments;
	std::vector<char*> _nonflags;
	std::string _program;

	bool IsFlag(char* arg) {
		if (arg[0] == '-') return true;
		return false;
	}
};

}  // namespace sarg
