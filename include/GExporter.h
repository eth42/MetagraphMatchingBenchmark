#pragma once
#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
#include <map>
#include "Tools.h"
#include "SimpleGraph.h"

namespace maxmatching {
	/* Parser from SimpleGraph to the .g file format.
	 * Each .g file consists of one row with the total vertex count
	 * and one line for each edge (not necessarily symmetric). */
	class GExporter {
	public:
		GExporter();
		~GExporter();

		void writeFile(std::string file, SimpleGraph<unsigned int>* g);
	};
}