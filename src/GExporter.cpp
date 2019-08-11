#include "GExporter.h"


namespace maxmatching {
	GExporter::GExporter() {}


	GExporter::~GExporter() {}

	void GExporter::writeFile(std::string file, SimpleGraph<unsigned int>* g) {
		Files::makePathToFile(file);
		auto out = std::ofstream(file);
		out << g->getVertexCount() << "\n";
		std::map<unsigned int, unsigned int> inverseMap;
		for (auto v : g->indexMap) {
			inverseMap[v.second] = v.first;
		}
		for (auto v : g->indexMap) {
			for (auto e : *(g->adjacencies[v.second])) {
				out << v.first << " " << inverseMap[e] << "\n";
			}
		}
		out.close();
	}
}