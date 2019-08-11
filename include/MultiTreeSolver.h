#pragma once
#include <vector>
#include <queue>
#include <algorithm>
#include <random>
#include <iostream>
#include <sstream>
#include "Solver.h"
#include "Statistics.h"
#include "Debug.h"
#include "CherryTree.h"
#include "Vertex.h"
#include "SolverEnums.h"

namespace maxmatching {
	template <class Label>
	class MultiTreeSolver : public Solver<Vertex<Label>, Label> {
	private:
		/* List of all vertices. Basically the graph representation. */
		std::vector<Vertex<Label>*> vertices;
		/* GrowQueue containing all vertices, from which growing is possible */
		List<Vertex<Label>> growQueue;
		/* Internal flag to force a restart of the algorithm. Should probably be
		 * removed in productive use and the caller should invoke the reset() function itself. */
		bool isCalculated;

		void preSort();
		void greedyPreSolve();
	public:
		/* Enum to describe which strategy to use for sorting the vertices
		 * prior to the algorithm. */
		PreSortStrategy preSortStrat;

		MultiTreeSolver();
		inline MultiTreeSolver(unsigned int arg1, unsigned int arg2) : MultiTreeSolver() {
			(void)arg1;
			(void)arg2;
		};
		~MultiTreeSolver();

		/* Auxiliary methods */
		std::vector<Vertex<Label>*>& getVertices();
		void addVertex(Vertex<Label>* v);
		void addEdge(Vertex<Label>* u, Vertex<Label>* v);
		std::vector<std::pair<Vertex<Label>*, Vertex<Label>*>>* getMatching();
		std::vector<Vertex<Label>*>* getMatchingRepresentatives();
		std::vector<std::pair<Label, Label>>* getMatchingLabels();
		void reset();
		//void shuffle(std::mt19937 g);

		/* Pseudocode implementations */
		void calculateMaxMatching();
		void growFrom(Vertex<Label>* v);
		void dissolveTwinTree(CherryTree<Label>* lTree, CherryTree<Label>* rTree);

		void clearVertices();
	};
}

#include "MultiTreeSolver.tpp"
