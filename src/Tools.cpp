#include "Tools.h"

namespace maxmatching {
	/* Time */
	unsigned long Time::currentTimeMillis() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	/* Strings */
	bool Strings::startsWith(const std::string& str, const std::string& prefix) {
		return str.compare(0, prefix.size(), prefix) == 0;
	}
	void Strings::removePrefix(const std::string& str, const std::string& prefix, std::string& ret) {
		std::stringstream rgxBuilder;
		rgxBuilder << "^\\s*" << prefix << "\\s*:\\s*((.*[^\\s])|)\\s*$";
		std::regex rgx(rgxBuilder.str());
		std::smatch match;
		if (std::regex_search(str.begin(), str.end(), match, rgx)) {
			ret = match[1];
		}
	}
	/* Parsing strings to int steppers */
	IntStepper Strings::parseIntStepper(const std::string& str) {
		return Strings::parseIntStepper(str, 1);
	}
	IntStepper Strings::parseIntStepper(const std::string& str, int defaultStep) {
		int min = 0, max = 0, step = defaultStep;
		std::istringstream stream(str);
		std::string token;
		if (std::getline(stream, token, ':')) {
			min = max = std::atoi(token.c_str());
			if (std::getline(stream, token, ':')) {
				max = std::atoi(token.c_str());
				if (std::getline(stream, token, ':')) {
					step = std::atoi(token.c_str());
				}
			}
		}
		return IntStepper(min, max, step);
	}

	/* Files */
	std::string Files::getFilename(const std::string& path) {
		return path.substr(path.find_last_of("/\\") + 1);
	}

	bool Files::isDir(const std::string & filename) {
		return boost::filesystem::is_directory(filename);
	}

	bool Files::isFile(const std::string & filename) {
		return boost::filesystem::exists(filename);
	}

	void Files::foreachFile(const std::string & dir, const std::function<void(std::string)> & func) {
		for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(dir), { })) {
			func(entry.path().generic_string());
		}
	}

	void Files::getContainedFiles(const std::string & dir, const std::string & pattern, std::vector<std::string> & ret) {
		std::regex rgx(pattern);
		for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(dir), { })) {
			auto file = entry.path().generic_string();
			if (std::regex_match(file, rgx, std::regex_constants::match_default)) {
				ret.push_back(file);
			}
		}
	}

	void Files::makePath(const std::string & dir) {
		std::string seperator = "/\\";
		int found = dir.find_first_of(seperator);
		while (found != (int)std::string::npos) {
			std::string parentDir = dir.substr(0, found);
			if (!isDir(parentDir)) {
				boost::filesystem::create_directory(parentDir);
			}
			found = dir.find_first_of(seperator, found + 1);
		}
		if (!isDir(dir)) {
			boost::filesystem::create_directory(dir);
		}
	}

	void Files::makePathToFile(const std::string & file) {
		Files::makePath(file.substr(0, file.find_last_of("/\\")));
	}
	/* Create the name for a buffer file given the name of a tsp file. */
	std::string Files::inputFileToBufferFile(const std::string & inputFile, const int& nNeighbors) {
		std::cmatch cm;
		std::regex_match(inputFile.c_str(), cm, std::regex("(.*\\/)*([^\\/\\\\]+)\\.[^\\/\\\\]+"));
		std::string input = cm[2];
		std::stringstream bufferFile;
		bufferFile << "assets/tmp/tsp/" << input << "_" << nNeighbors << ".g";
		return bufferFile.str();
	}

	/* Random */
	std::mt19937_64* Random::makeRandom(const long& seed) {
		return new std::mt19937_64(seed);
	}
}
