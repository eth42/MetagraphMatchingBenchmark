#pragma once
/* Boost is in a terrible state with deprecation all over it :| */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <algorithm>
#include <cstring>
#include "boost/iostreams/device/mapped_file.hpp"
#include "SimpleGraph.h"

namespace maxmatching {
	/* Parser from the .g file format to SimpleGraph.
	* Each .g file consists of one row with the total vertex count
	* and one line for each edge (not necessarily symmetric). */
	class GImporter {
	public:
		GImporter();
		~GImporter();

		SimpleGraph<unsigned int>* importFile(const std::string& file);
	};
}