#pragma once
/* Boost is in a terrible state with deprecation all over it :| */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <algorithm>
#include <array>
#include "boost/graph/random.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/undirected_graph.hpp"
#include "SimpleGraph.h"
#include "DimacsGenerator.h"
#ifdef HAS_FADE
#include "TriangleGenerator.h"
#endif
#include "Tools.h"

namespace maxmatching {
	/* Implements all graph generation algorithms considered for benchmarking */
	class GGenerator {
	private:
		template <typename VType>
		SimpleGraph<unsigned int>* createRandomEuclid(unsigned int nVertices, unsigned int nNeighbors, std::mt19937_64 &rgen);
	public:
		GGenerator();
		~GGenerator();

		SimpleGraph<unsigned int>* createRandomBoost(unsigned int nVertices, unsigned int nNeighbors, std::mt19937_64 &rgen);
		SimpleGraph<unsigned int>* createRandomDimacs(unsigned int nVertices, unsigned int nNeighbors, std::mt19937_64 &rgen);
#ifdef HAS_FADE
		SimpleGraph<unsigned int>* createRandomTriangle(unsigned int nVertices, std::mt19937_64 &rgen);
#endif
#define DECLARE_RANDOM_EUCLID(DIM) \
		SimpleGraph<unsigned int>* createRandomEuclid ## DIM ## d(unsigned int nVertices, unsigned int nNeighbors, std::mt19937_64 &rgen);
		DECLARE_RANDOM_EUCLID(2);
		DECLARE_RANDOM_EUCLID(3);
		DECLARE_RANDOM_EUCLID(10);
		DECLARE_RANDOM_EUCLID(20);
#undef DECLARE_RANDOM_EUCLID
		SimpleGraph<unsigned int>* createWorstCaseGabow(unsigned int m);
		SimpleGraph<unsigned int>* createTrianglesA(unsigned int nTriangles);
		SimpleGraph<unsigned int>* createTrianglesB(unsigned int nTriangles);
		SimpleGraph<unsigned int>* createHoneyCombs(unsigned int width, unsigned int rows);
		SimpleGraph<unsigned int>* createHoneyCombsPlus(unsigned int width, unsigned int rows);
		SimpleGraph<unsigned int>* createHoneyCombsCaps(unsigned int width, unsigned int rows);
		SimpleGraph<unsigned int>* createHoneyCombsInner(unsigned int width, unsigned int rows);
	};

	/* Create a nearest neighbor graph for any dimension using the euclidean norm */
	template <typename VType>
	SimpleGraph<unsigned int>* GGenerator::createRandomEuclid(unsigned int nVertices, unsigned int nNeighbors, std::mt19937_64 &rgen) {
		auto ret = new SimpleGraph<unsigned int>();
		std::vector<VType> coords(nVertices);
		for (unsigned int i = 0; i < nVertices; i++) {
			VType v;
			for (unsigned int d = 0; d < v.size(); d++) {
				v[d] = rgen();
			}
			coords[i] = v;
			ret->addVertex(i);
		}
		for (unsigned int i = 0; i < nVertices; i++) {
			std::vector<double> dists(nVertices);
			for (unsigned int j = 0; j < nVertices; j++) {
				dists[j] = Vectors<VType>::distance(coords[i], coords[j]);
			}
			std::vector<double> distsCpy(dists);
			std::sort(distsCpy.begin(), distsCpy.end());
			for (unsigned int j = 0; j < nVertices; j++) {
				if (i != j && dists[j] <= distsCpy[nNeighbors]) {
					ret->addEdgeSym(i, j);
				}
			}
		}
		ret->cleanUp();
		return ret;
	}
}