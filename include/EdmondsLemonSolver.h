#pragma once
#include <map>
#include <vector>
#include <tuple>
#include "lemon/list_graph.h"
#include "lemon/matching.h"
#include "Solver.h"
#include "EdmondsVertex.h"

namespace maxmatching {
	/* Wrapper class for the lemon implementation of Edmonds' algorithm */
	template <class Label>
	class EdmondsLemonSolver : public Solver<EdmondsVertex<Label>, Label> {
	private:
		typedef lemon::ListGraph Graph;
		typedef lemon::MaxMatching<Graph> Matcher;
		/* List of all vertices. */
		std::vector<EdmondsVertex<Label>*> vertices;
		/* Map to store the vertex representation of lemon */
		std::map < EdmondsVertex<Label>*, Graph::Node> vertexMap;
		/* Map to create the matching list from the solution provided by lemon */
		std::map < Graph::Node, EdmondsVertex<Label>*> inverseVertexMap;
		/* The graph representation used by lemon */
		Graph* graph;
		/* The solver object of the lemon implementation */
		Matcher* matcher;
	public:
		EdmondsLemonSolver();
		inline EdmondsLemonSolver(unsigned int arg1, unsigned int arg2) : EdmondsLemonSolver() {
			(void)arg1;
			(void)arg2;
		};
		~EdmondsLemonSolver();

		void addVertex(EdmondsVertex<Label>* v);
		void addEdge(EdmondsVertex<Label>* u, EdmondsVertex<Label>* v);
		void calculateMaxMatching();
		std::vector<EdmondsVertex<Label>*>* getMatchingRepresentatives();
		std::vector<std::pair<Label, Label>>* getMatchingLabels();
		void reset();
		void clearVertices();
	};

}

#include "EdmondsLemonSolver.tpp"
