#include "GImporter.h"

namespace maxmatching {
	GImporter::GImporter() {}

	GImporter::~GImporter() {}

	SimpleGraph<unsigned int>* GImporter::importFile(const std::string& file) {
		auto ret = new SimpleGraph<unsigned int>();
		boost::iostreams::mapped_file mmap(file, boost::iostreams::mapped_file::readonly);
		auto begin = mmap.const_data();
		auto end = begin + mmap.size();
		auto it = begin;
		if ((it = static_cast<const char*>(memchr(it, '\n', end - it)))) {
			std::string line(begin, it);
			unsigned int nvs = std::atoi(line.c_str());
			for (unsigned int i = 0; i < nvs; i++) {
				ret->addVertex(i);
			}
		}
		for (it++; it < end; it++) {
			auto lineBegin = it;
			if ((it = static_cast<const char*>(memchr(it, '\n', end - it)))) {
				std::string line(lineBegin, it);
				if (line.size() == 0) {
					continue;
				}
				int from, to;
				std::sscanf(line.c_str(), "%u %u", &from, &to);
				ret->addEdgeSym(from, to);
			}
		}
		mmap.close();
		ret->cleanUp();
		return ret;
	}
}