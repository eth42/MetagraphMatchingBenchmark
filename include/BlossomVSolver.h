#pragma once
#include "Solver.h"
#include "BlossomVertex.h"
#include <map>
#include <vector>
#include <iostream>
#include "PerfectMatching.h"

namespace maxmatching {
	/* Wrapper class for the boost implementation of the Blossom IV algorithm */
	template <class Label>
	class BlossomVSolver : public Solver<BlossomVertex<Label>, Label> {
	private:
		/* List of all vertices. */
		std::vector<BlossomVertex<Label>*> vertices;
		std::vector<std::pair< BlossomVertex<Label>*, BlossomVertex<Label>*>> edges;
		std::vector<std::pair< BlossomVertex<Label>*, BlossomVertex<Label>*>> matches;
		std::vector<unsigned int> edgeWeights;
		bool duplicateInstance;
	public:
		BlossomVSolver();
		inline BlossomVSolver(unsigned int arg1, unsigned int arg2)
			: vertices()
			, edges()
			, matches()
			, duplicateInstance(arg1 > 0) {
			(void)arg2;
		};
		~BlossomVSolver();

		void addVertex(BlossomVertex<Label>* v);
		void addEdge(BlossomVertex<Label>* u, BlossomVertex<Label>* v);
		void calculateMaxMatching();
		std::vector<BlossomVertex<Label>*>* getMatchingRepresentatives();
		std::vector<std::pair<Label, Label>>* getMatchingLabels();
		void reset();
		void clearVertices();
	};

}

#include "BlossomVSolver.tpp"
