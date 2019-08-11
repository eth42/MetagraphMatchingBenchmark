#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <random>

namespace maxmatching {
	/* Base class to buffer graphs in a uniform representation
	 * before feeding it to the different solvers */
	template<class Label>
	class SimpleGraph {
	private:
		unsigned int indexCtr;
		static unsigned int prtCtr;
	public:
		std::vector<Label> vertexLabel;
		std::map<Label, unsigned int> indexMap;
		std::map<unsigned int, std::vector<unsigned int>*> adjacencies;

		SimpleGraph();
		~SimpleGraph();

		void addVertex(Label v);
		void addEdge(Label start, Label end);
		void addEdgeSym(Label start, Label end);
		void addEdgeSymSafe(Label start, Label end);
		void cleanUp();

		void shuffle(std::mt19937_64 g);

		unsigned int getVertexCount();
		unsigned int getEdgeCount();
		void print();
	};
}

#include "SimpleGraph.tpp"