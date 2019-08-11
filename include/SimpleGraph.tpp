#include "SimpleGraph.h"

namespace maxmatching {
	template<class Label>
	unsigned int SimpleGraph<Label>::prtCtr = 0;

	template<class Label>
	SimpleGraph<Label>::SimpleGraph()
		: indexCtr(0)
		, vertexLabel()
		, indexMap()
		, adjacencies() {}

	template<class Label>
	SimpleGraph<Label>::~SimpleGraph() {
		for (auto pair : this->adjacencies) {
			delete(pair.second);
		}
	}


	template<class Label>
	void SimpleGraph<Label>::addVertex(Label v) {
		this->vertexLabel.push_back(v);
		auto index = indexCtr++;
		this->indexMap[v] = index;
		this->adjacencies[index] = new std::vector<unsigned int>();
	}

	template<class Label>
	void SimpleGraph<Label>::addEdge(Label start, Label end) {
		auto u = indexMap[start];
		auto v = indexMap[end];
		auto uList = this->adjacencies[u];
		uList->push_back(v);
	}

	template<class Label>
	void SimpleGraph<Label>::addEdgeSym(Label start, Label end) {
		auto u = indexMap[start];
		auto v = indexMap[end];
		auto uList = this->adjacencies[u];
		uList->push_back(v);
		auto vList = this->adjacencies[v];
		vList->push_back(u);
	}

	template<class Label>
	void SimpleGraph<Label>::addEdgeSymSafe(Label start, Label end) {
		auto u = indexMap[start];
		auto v = indexMap[end];
		auto uList = this->adjacencies[u];
		if (std::find(uList->begin(), uList->end(), v) == uList->end()) {
			uList->push_back(v);
		}
		auto vList = this->adjacencies[v];
		if (std::find(vList->begin(), vList->end(), u) == vList->end()) {
			vList->push_back(u);
		}
	}

	/* Removes duplicate entries in the adjacency lists. */
	template<class Label>
	void SimpleGraph<Label>::cleanUp() {
		for (auto it = this->indexMap.begin(); it != this->indexMap.end(); it++) {
			unsigned int v = it->second;
			std::vector<unsigned int>* vns = this->adjacencies[v];
			std::sort(vns->begin(), vns->end());
			vns->erase(std::unique(vns->begin(), vns->end()), vns->end());
		}
	}

	/* Shuffles the vertex list and the adjacency lists, thus creating an isomorphic graph. */
	template<class Label>
	void SimpleGraph<Label>::shuffle(std::mt19937_64 g) {
		std::vector<unsigned int> rename(this->indexCtr);
		for (unsigned int i = 0; i < this->indexCtr; i++) {
			rename[i] = i;
		}
		std::shuffle(rename.begin(), rename.end(), g);
		for (auto it = this->indexMap.begin(); it != this->indexMap.end(); it++) {
			this->indexMap[it->first] = rename[it->second];
		}
		std::map<unsigned int, std::vector<unsigned int>*> newAdjacencies;
		for (auto it = this->adjacencies.begin(); it != this->adjacencies.end(); it++) {
			auto adjList = it->second;
			for (unsigned int i = 0; i < adjList->size(); i++) {
				(*adjList)[i] = rename[(*adjList)[i]];
			}
			std::shuffle(adjList->begin(), adjList->end(), g);
			newAdjacencies[rename[it->first]] = adjList;
		}
		this->adjacencies = newAdjacencies;
	}


	template<class Label>
	unsigned int SimpleGraph<Label>::getVertexCount() {
		return this->indexMap.size();
	}

	template<class Label>
	unsigned int SimpleGraph<Label>::getEdgeCount() {
		unsigned int ret = 0;
		for (auto pair : this->adjacencies) {
			ret += pair.second->size();
		}
		return ret / 2;
	}

	/* Prints the graph as both a .dot and a .gml file for debugging purposes. */
	template<class Label>
	void SimpleGraph<Label>::print() {
		std::stringstream gmlStream, dotStream;
		gmlStream << "graph [\n";
		dotStream << "digraph G {\n";
		for (auto it = this->adjacencies.begin(); it != this->adjacencies.end(); it++) {
			unsigned int v = it->first;
			gmlStream << "\tnode [\n\t\tid " << v << "\n\t\tlabel \"" << v << "\"\n\t]\n";
			auto adjList = it->second;
			for (unsigned int i = 0; i < adjList->size(); i++) {
				unsigned int n = (*adjList)[i];
				gmlStream << "\tedge [\n\t\tsource " << v << "\n\t\ttarget " << n << "\n\t]\n";
				dotStream << "\t" << v << " -> " << n << ";\n";
			}
		}
		gmlStream << "]\n";
		dotStream << "}\n";
		std::string gmlBuffer = gmlStream.str(), dotBuffer = dotStream.str();
		std::ofstream gmlWriter, dotWriter;
		std::stringstream gmlFileNameBuilder, dotFileNameBuilder;
		gmlFileNameBuilder << "debug/graphs/sg" << prtCtr << ".gml";
		dotFileNameBuilder << "debug/graphs/sg" << prtCtr << ".dot";
		gmlWriter.open(gmlFileNameBuilder.str());
		dotWriter.open(dotFileNameBuilder.str());
		if (!gmlWriter.fail() && !dotWriter.fail()) {
			gmlWriter << gmlBuffer;
			dotWriter << dotBuffer;
		} else {
		}
		gmlWriter.close();
		dotWriter.close();
		std::cout << "Printed base graph to files '"
			<< gmlFileNameBuilder.str() << "' and '"
			<< dotFileNameBuilder.str() << "'.\n";
		prtCtr++;
	}
}
