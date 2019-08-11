#pragma once
/* Boost is in a terrible state with deprecation all over it :| */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <chrono>
#include <regex>
#include <cmath>
#include <sstream>
#include <fstream>
#include <functional>
#include <random>
#include "boost/filesystem.hpp"
#include "boost/range/iterator_range.hpp"
#include "IntStepper.h"

/* Compact writing for static classes */
#define L_MAKE_STATIC_T(TEMPLATES, NAME, FUNCDEFS) TEMPLATES \
/**/class NAME { \
/**/private: \
/**/	inline NAME() {} \
/**/	inline ~NAME() {} \
/**/public: \
/**/	FUNCDEFS \
/**/};
#define L_MAKE_STATIC(NAME, FUNCDEFS) L_MAKE_STATIC_T(,NAME,FUNCDEFS)

/* This file contains static helper functions for getting current timestamps
 * and working with strings and vectors. */

namespace maxmatching {
	L_MAKE_STATIC(Time, \
		static unsigned long currentTimeMillis();
	);

	L_MAKE_STATIC(Strings, \
		static bool startsWith(const std::string& str, const std::string& prefix); \
		static void removePrefix(const std::string& str, const std::string& prefix, std::string& ret); \
		static IntStepper parseIntStepper(const std::string& str); \
		static IntStepper parseIntStepper(const std::string& str, int defaultStep);
	);

	L_MAKE_STATIC_T(template<typename VType>, Vectors,
		inline static double distance(VType& from, VType& to) {
		VType diff;
		for (unsigned int i = 0; i < from.size(); i++) {
			diff[i] = from[i] - to[i];
		}
		return length(diff);
	}
	inline static double length(VType v) {
		double squaredSum = 0;
		for (unsigned int i = 0; i < v.size(); i++) {
			squaredSum += v[i] * v[i];
		}
		return std::sqrt(squaredSum);
	}
	);

	L_MAKE_STATIC(Files,
		static std::string getFilename(const std::string& path);
	static bool isDir(const std::string& filename);
	static bool isFile(const std::string& filename);
	static void foreachFile(const std::string& dir, const std::function<void(std::string)>& func);
	static void getContainedFiles(const std::string& dir, const std::string& pattern, std::vector<std::string>& ret);
	static void makePath(const std::string& dir);
	static void makePathToFile(const std::string& file);
	static std::string inputFileToBufferFile(const std::string& inputFile, const int& nNeighbors);
	);

	L_MAKE_STATIC(Random,
		static std::mt19937_64* makeRandom(const long& seed);
	);
}

#undef L_MAKE_STATIC
#undef L_MAKE_STATIC_T
