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
#include "wr/MCherryTree.h"
#include "wr/MVertex.h"
#include "SolverEnums.h"

namespace maxmatching {
namespace wr {
	template <class Label>
	class MetaGraphsSolver : public Solver<MVertex<Label>, Label> {
	private:
		/* List of all vertices. Basically the graph representation. */
		std::vector<MVertex<Label>*> vertices;
		/* List of remaining search trees. */
		List<MCherryTree<Label>> remainingTrees;
		/* GrowQueue containing all vertices, from which growing is possible */
		List<MVertex<Label>> growQueue;
		/* waitingRoom containing all vertices, from which growing is possible,
		 * but delayed due to too many metaedges incident to the metavertex of
		 * the containing cherrytree */
		List<MVertex<Label>> waitingRoom;
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

		void preSort();
		void greedyPreSolve();
	public:
		/* Enum to describe which strategy to use for sorting the vertices
		 * prior to the algorithm. */
		PreSortStrategy preSortStrat;

		MetaGraphsSolver();
		explicit MetaGraphsSolver(unsigned int maxMetaNeighbors, unsigned int arg2);
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
		void growFrom(MVertex<Label>* v);
		void applyMetaMatching(std::vector<MVertex<Label>*>* matching);
		void batchDissolveTrees(List<MCherryTree<Label>>* twinTrees);
		void clearWaitingRoom();

		/* Benchmarks */
		double getI();
		double getRI();

		void clearVertices();
	};
}
}
#include "wr/MetaGraphsSolver.tpp"
