#pragma once
/* Boost is in a terrible state with deprecation all over it :| */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "Solver.h"
#include "EdmondsVertex.h"
#include <map>
#include <vector>
#include <iostream>
#include "boost/property_map/property_map.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/labeled_graph.hpp"
#include "boost/graph/max_cardinality_matching.hpp"
#include "boost/graph/iteration_macros.hpp"

namespace maxmatching {
	/* Wrapper class for the boost implementation of Edmonds' algorithm */
	template <class Label>
	class EdmondsBoostSolver : public Solver<EdmondsVertex<Label>, Label> {

		typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> AdjList;
		typedef boost::labeled_graph<AdjList, EdmondsVertex<Label>*, boost::mapS> Graph;
		typedef typename boost::graph_traits<Graph>::vertex_descriptor VHandle;
		typedef std::map<VHandle, VHandle> MateMap;
		typedef boost::associative_property_map<MateMap> MateMapW;

	private:
		/* List of all vertices. */
		std::vector<EdmondsVertex<Label>*> vertices;
		/* Map to create the matching list from the solution provided by boost */
		std::map<VHandle, EdmondsVertex<Label>*> invertedVMap;
		/* The graph representation used by boost */
		Graph graph;
		/* Buffer object to store the boost solution */
		MateMap mateMap;
	public:
		EdmondsBoostSolver();
		inline EdmondsBoostSolver(unsigned int arg1, unsigned int arg2) : EdmondsBoostSolver() {
			(void)arg1;
			(void)arg2;
		};
		~EdmondsBoostSolver();

		void addVertex(EdmondsVertex<Label>* v);
		void addEdge(EdmondsVertex<Label>* u, EdmondsVertex<Label>* v);
		void calculateMaxMatching();
		std::vector<EdmondsVertex<Label>*>* getMatchingRepresentatives();
		std::vector<std::pair<Label, Label>>* getMatchingLabels();
		void reset();
		void clearVertices();
	};

}

#include "EdmondsBoostSolver.tpp"
