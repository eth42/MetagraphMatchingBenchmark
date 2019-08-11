#pragma once
#define METAGRAPHSSOLVER_H
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include "Debug.h"
#include "Solver.h"
#include "Statistics.h"
#include "List.h"
#include "ListElement.h"
#include "qpt/MCherryTree.h"
#include "qpt/MVertex.h"
#include "SolverEnums.h"

namespace maxmatching {
namespace qpt {
	template <class Label>
	class MetaGraphsSolver : public Solver<MVertex<Label>, Label> {
	private:
		/* List of all vertices. Basically the graph representation. */
		std::vector<MVertex<Label>*> vertices;
		/* List of frustrated search trees. */
		List<MCherryTree<Label>> frustratedTrees;
		/* Stores all trees, from which growing is possible, in layers by number of meta edges */
		List<MCherryTree<Label>>** growQueueStack;
		/* Stores all trees, from which growing is not possible, but which contain potential new blossoms, in layers by number of meta edges */
		List<MCherryTree<Label>>** frustratedShrinkableStack;
		/* Contains information which metavertices are connected via an edge,
		 * so metaedges are not created twice between the same metavertices. */
		std::vector<bool> metaEdgeMatrix;
		/* Internal flag to force a restart of the algorithm. Should probably be
		 * removed in productive use and the caller should invoke the reset() function itself. */
		bool isCalculated;

		/* Coefficients for benchmarking */
		double I;
		double RI;

		/* Stop growing of cherry trees once a maximum number of meta neighbors is achieved */
		unsigned int maxMetaNeighbors;
		unsigned int currentMinNeighbors;
		unsigned int maxMetaNeighborsShrinking;
		unsigned int currentMinNeighborsShrinking;
		unsigned long nUnmatchedNodes;
		unsigned int metaEdges;

		void preSort();
		void greedyPreSolve();
		void storeInCorrectList(ListElement<MCherryTree<Label>>* treeEl);
	public:
		/* Enum to describe which strategy to use for sorting the vertices
		 * prior to the algorithm. */
		PreSortStrategy preSortStrat;

		MetaGraphsSolver();
		explicit MetaGraphsSolver(unsigned int maxMetaNeighbors, unsigned int maxMetaNeighborsShrinking);
		~MetaGraphsSolver();

		/* Auxiliary methods */
		std::vector<MVertex<Label>*>& getVertices();
		void addVertex(MVertex<Label>* v);
		void addEdge(MVertex<Label>* u, MVertex<Label>* v);
		void addMetaEdge(MVertex<Label>* u, MVertex<Label>* v);
		void addEdge(MVertex<Label>* u, MVertex<Label>* v, HalfEdge<Label>* label);
		void addMetaEdge(MVertex<Label>* u, MVertex<Label>* v, HalfEdge<Label>* label);
		std::vector<HalfEdge<Label>*>* getMatching();
		std::vector<MVertex<Label>*>* getMatchingRepresentatives();
		std::vector<std::pair<Label, Label>>* getMatchingLabels();
		void reset();

		/* Pseudocode implementations */
		void calculateMaxMatching();
		ListElement<MCherryTree<Label>>* getGrowableTreeEl();
		void growFrom(MVertex<Label>* v);
		void applyMetaMatching(std::vector<MVertex<Label>*>* matching);
		void batchDissolveTrees(List<MCherryTree<Label>>* twinTrees);

		/* Benchmarks */
		double getI();
		double getRI();

		void clearVertices();
	};
}
}
#include "qpt/MetaGraphsSolver.tpp"
