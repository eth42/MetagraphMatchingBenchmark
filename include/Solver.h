#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include "SimpleGraph.h"

namespace maxmatching {
	/* Base class for solvers. Solvers use the added vertices.
	 * Adding edges is only a comfort function, which makes the necessary calls
	 * on the vertices. Added edges will thus survive in the vertices upon
	 * deletion of the solver. To delete all vertices and edges, and thus
	 * avoid memory leaks, use the clear vertices method. */
	template <class VClass, class LClass>
	class Solver {
	public:
		inline Solver() {};
		inline Solver(unsigned int arg1, unsigned int arg2): Solver() {};
		inline virtual ~Solver() {};

		/* Parse a SimpleGraph into the used graph type. */
		inline void readGraph(SimpleGraph<LClass>* g) {
			VClass** vs = new VClass * [g->getVertexCount()];
			for (auto it = g->vertexLabel.begin(); it != g->vertexLabel.end(); it++) {
				auto vLabel = *it;
				auto vIndex = g->indexMap[vLabel];
				VClass* v = new VClass(vLabel);
				vs[vIndex] = v;
				this->addVertex(v);
			}
			for (auto it = g->vertexLabel.begin(); it != g->vertexLabel.end(); it++) {
				auto vLabel = *it;
				auto vIndex = g->indexMap[vLabel];
				auto v = vs[vIndex];
				auto adjList = g->adjacencies[vIndex];
				for (auto it2 = adjList->begin(); it2 != adjList->end(); it2++) {
					auto wIndex = *it2;
					auto w = vs[wIndex];
					if (v->id < w->id) {
						this->addEdge(v, w);
					}
				}
			}
			delete[](vs);
		}

		virtual void addVertex(VClass* v) = 0;
		virtual void addEdge(VClass* u, VClass* v) = 0;
		virtual void calculateMaxMatching() = 0;
		virtual std::vector<VClass*>* getMatchingRepresentatives() = 0;
		virtual std::vector<std::pair<LClass, LClass>>* getMatchingLabels() = 0;
		virtual void reset() = 0;
		virtual void clearVertices() = 0;
	};
}
