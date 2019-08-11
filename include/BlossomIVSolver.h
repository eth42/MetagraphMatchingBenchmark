#pragma once
#include "Solver.h"
#include "BlossomVertex.h"
#include <map>
#include <vector>
#include <iostream>

namespace maxmatching {
	/* Wrapper class for the boost implementation of the Blossom IV algorithm */
	template <class Label>
	class BlossomIVSolver : public Solver<BlossomVertex<Label>, Label> {
	private:
		/* List of all vertices. */
		std::vector<BlossomVertex<Label>*> vertices;
		std::vector<std::pair< BlossomVertex<Label>*, BlossomVertex<Label>*>> edges;
		std::vector<unsigned int> edgeWeights;
		int* matches;
		int n_matches;
		bool duplicateInstance;
	public:
		BlossomIVSolver();
		inline explicit BlossomIVSolver(unsigned int arg1, unsigned int arg2)
			: vertices()
			, edges()
			, edgeWeights()
			, matches(nullptr)
			, n_matches(0)
			, duplicateInstance(arg1>0){
			(void)arg2;
		};
		~BlossomIVSolver();

		void addVertex(BlossomVertex<Label>* v);
		void addEdge(BlossomVertex<Label>* u, BlossomVertex<Label>* v);
		void calculateMaxMatching();
		std::vector<BlossomVertex<Label>*>* getMatchingRepresentatives();
		std::vector<std::pair<Label, Label>>* getMatchingLabels();
		void reset();
		void clearVertices();
	};

}

#include "BlossomIVSolver.tpp"
