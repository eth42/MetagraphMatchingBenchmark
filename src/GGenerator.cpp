#include "GGenerator.h"


namespace maxmatching {
	GGenerator::GGenerator() {}
	GGenerator::~GGenerator() {}

	SimpleGraph<unsigned int>* GGenerator::createRandomBoost(unsigned int nVertices, unsigned int nNeighbors, std::mt19937_64 &rgen) {
		auto ret = new SimpleGraph<unsigned int>();
		typedef boost::undirected_graph<> Graph;
		typedef boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
		//typedef boost::graph_traits < Graph >::adjacency_iterator AdjIterator;
		Graph boostGraph;
		boost::generate_random_graph(boostGraph, nVertices, nVertices * nNeighbors, rgen);
		IndexMap index = boost::get(boost::vertex_index, boostGraph);
		auto vertices = boost::vertices(boostGraph);
		for (; vertices.first != vertices.second; vertices.first++) {
			ret->addVertex(index[*vertices.first]);
		}
		auto edges = boost::edges(boostGraph);
		for (; edges.first != edges.second; edges.first++) {
			auto e = *edges.first;
			ret->addEdgeSym(index[e.m_source], index[e.m_target]);
		}
		ret->cleanUp();
		return ret;
	}

	SimpleGraph<unsigned int>* GGenerator::createRandomDimacs(unsigned int nVertices, unsigned int nNeighbors, std::mt19937_64 &rgen) {
		DimacsGenerator dg;
		return dg.generate(nVertices, nVertices * nNeighbors, rgen);
	}

#ifdef HAS_FADE
	SimpleGraph<unsigned int>* GGenerator::createRandomTriangle(unsigned int nVertices, std::mt19937_64 &rgen) {
		TriangleGenerator tg;
		return tg.generate(nVertices, rgen);
	}
#endif

#define MAKE_RANDOM_EUCLID(DIM) \
/**/SimpleGraph<unsigned int>* GGenerator::createRandomEuclid ## DIM ## d(unsigned int nVertices, unsigned int nNeighbors, std::mt19937_64 &rgen) { \
/**/	typedef std::array<double, DIM> tuple; \
/**/	return this->createRandomEuclid<tuple>(nVertices, nNeighbors, rgen); \
/**/}
	MAKE_RANDOM_EUCLID(2);
	MAKE_RANDOM_EUCLID(3);
	MAKE_RANDOM_EUCLID(10);
	MAKE_RANDOM_EUCLID(20);
#undef MAKE_RANDOM_EUCLID

	/* Implementation of the worst case graph proposed by Gabow
	 * in "Efficient Implementation of Edmonds' Algorithm for
	 * Maximum Matching on Graphs". The last vertex is moved to the front
	 * to create the worst case described in the thesis. */
	SimpleGraph<unsigned int>* GGenerator::createWorstCaseGabow(unsigned int m) {
		auto ret = new SimpleGraph<unsigned int>();
		ret->addVertex(0);
		/* Init 6m vertices */
		for (unsigned int i = 1; i <= 6 * m; i++) {
			ret->addVertex(i);
		}
		/* Make complete subgraph K_{4m} */
		for (unsigned int i = 1; i <= 4 * m; i++) {
			/* Edges are symmetric anyway, it suffices to add "forward" edges */
			for (unsigned int j = i + 1; j <= 4 * m; j++) {
				ret->addEdgeSym(i, j);
			}
		}
		/* Connect the K_{4m} to vertices 4m+1, ..., 6m */
		for (unsigned int i = 1; i < 2 * m; i++) {
			ret->addEdgeSym(2 * i - 1, 4 * m + i);
		}
		ret->addEdgeSym(4 * m - 1, 0);
		return ret;
	}

	/* Creates a series of triangles (A_i,B_i,C_i)_i connected by
	 * edges (X_i,X_{i+1}) exist for any X in {A,B,C} */
	SimpleGraph<unsigned int>* GGenerator::createTrianglesA(unsigned int nTriangles) {
		auto ret = new SimpleGraph<unsigned int>();
		/* Init 3*nTriangles vertices */
		for (unsigned int i = 0; i < 3 * nTriangles; i++) {
			ret->addVertex(i);
		}
		/* Create triangles */
		for (unsigned int i = 0; i < nTriangles; i++) {
			ret->addEdgeSym(3 * i, 3 * i + 1);
			ret->addEdgeSym(3 * i, 3 * i + 2);
			ret->addEdgeSym(3 * i + 1, 3 * i + 2);
		}
		/* Connect triangles */
		for (unsigned int i = 0; i < nTriangles - 1; i++) {
			for (unsigned int j = 0; j < 3; j++) {
				ret->addEdgeSym(3 * i + j, 3 * i + j + 3);
			}
		}
		return ret;
	}

	/* Creates a series of triangles (A_i,B_i,C_i)_i connected by
	 * edges (A_i,A_{i+1}) iff i is even and (B_i,B_{i+1}) iff i is odd */
	SimpleGraph<unsigned int>* GGenerator::createTrianglesB(unsigned int nTriangles) {
		auto ret = new SimpleGraph<unsigned int>();
		/* Init 3*nTriangles vertices */
		for (unsigned int i = 0; i < 3 * nTriangles; i++) {
			ret->addVertex(i);
		}
		/* Create triangles */
		for (unsigned int i = 0; i < nTriangles; i++) {
			ret->addEdgeSym(3 * i, 3 * i + 1);
			ret->addEdgeSym(3 * i, 3 * i + 2);
			ret->addEdgeSym(3 * i + 1, 3 * i + 2);
		}
		/* Connect triangles */
		for (unsigned int i = 0; i < nTriangles - 1; i++) {
			if (i % 2 == 0) {
				ret->addEdgeSym(3 * i, 3 * i + 3);
			} else {
				ret->addEdgeSym(3 * i + 1, 3 * i + 4);
			}
		}
		return ret;
	}

	/* Creates a honey comb like structure, i.e. an array of hexagons.
	 * It consists of a series of two rows with witdh nodes,
	 * followed by two rows with width + 1 nodes. The parameters rows however
	 * describes the number of comb rows, which means (rows*3+1) effective vertex rows. */
	SimpleGraph<unsigned int>* GGenerator::createHoneyCombs(unsigned int width, unsigned int rows) {
		auto ret = new SimpleGraph<unsigned int>();
		/* Init vertices */
		unsigned int i = 0;
		for (i = 0; i < (rows + 1) * (2 * width + 1); i++) {
			ret->addVertex(i);
		}
		/* Create combs */
		i = width;
		for (unsigned int vRow = 1; vRow < (rows + 1) * 2; vRow++) {
			bool isWideRow = ((vRow - 1) / 2) % 2 == 0;
			bool isPeakRow = vRow % 2 == 0;
			unsigned int vRowWidth = width + (isWideRow ? 1 : 0);
			for (unsigned int vOffset = 0; vOffset < vRowWidth; vOffset++) {
				if (isPeakRow) {
					ret->addEdgeSym(i + vOffset, i - vRowWidth + vOffset);
				} else {
					if (isWideRow) {
						if (vOffset > 0) {
							ret->addEdgeSym(i + vOffset, i - vRowWidth + vOffset);
						}
						if (vOffset < vRowWidth - 1) {
							ret->addEdgeSym(i + vOffset, i - vRowWidth + 1 + vOffset);
						}
					} else {
						ret->addEdgeSym(i + vOffset, i - vRowWidth - 1 + vOffset);
						ret->addEdgeSym(i + vOffset, i - vRowWidth + vOffset);
					}
				}
			}
			i += vRowWidth;
		}
		return ret;
	}

	/* Same as honey combs, except that every second outer vertex
	 * has and additional neighbor. */
	SimpleGraph<unsigned int>* GGenerator::createHoneyCombsPlus(unsigned int width, unsigned int rows) {
		auto ret = createHoneyCombs(width, rows);
		unsigned int startingSize = ret->getVertexCount();
		/* Index for newly appended vertices */
		unsigned int appendixCtr = ret->getVertexCount();
		/* Create side appendices */
		unsigned int outerVertex = width;
		for (unsigned int vRow = 1; vRow < (rows + 1) * 2 - 1; vRow++) {
			bool isWideRow = ((vRow - 1) / 2) % 2 == 0;
			bool isPeakRow = vRow % 2 == 0;
			unsigned int vRowWidth = width + (isWideRow ? 1 : 0);
			if (!isPeakRow) {
				ret->addVertex(appendixCtr);
				ret->addEdgeSym(appendixCtr++, outerVertex);
				ret->addVertex(appendixCtr);
				ret->addEdgeSym(appendixCtr++, outerVertex + vRowWidth - 1);
			}
			outerVertex += vRowWidth;
		}
		/* Create top appendices */
		for (outerVertex = 1; outerVertex < width; outerVertex += 2) {
			ret->addVertex(appendixCtr);
			ret->addEdgeSym(appendixCtr++, outerVertex);
		}
		/* Create bottom appendices */
		for (outerVertex = startingSize - width; outerVertex < startingSize; outerVertex += 2) {
			ret->addVertex(appendixCtr);
			ret->addEdgeSym(appendixCtr++, outerVertex);
		}
		return ret;
	}

	/* Same as honey combs plus, except that the upper and lower peaks are connected,
	 * thus creating numerous possibilities for odd circles. */
	SimpleGraph<unsigned int>* GGenerator::createHoneyCombsCaps(unsigned int width, unsigned int rows) {
		auto ret = createHoneyCombs(width, rows);
		unsigned int startingSize = ret->getVertexCount();
		/* Index for newly appended vertices */
		unsigned int appendixCtr = ret->getVertexCount();
		unsigned int outerVertex = width;
		/* Create top caps */
		for (outerVertex = 1; outerVertex < width; outerVertex++) {
			ret->addEdgeSym(outerVertex - 1, outerVertex);
		}
		/* Create bottom caps */
		for (outerVertex = startingSize - width + 1; outerVertex < startingSize; outerVertex++) {
			ret->addEdgeSym(outerVertex - 1, outerVertex);
		}
		/* Create side appendices */
		outerVertex = width;
		for (unsigned int vRow = 1; vRow < (rows + 1) * 2 - 1; vRow++) {
			bool isWideRow = ((vRow - 1) / 2) % 2 == 0;
			bool isPeakRow = vRow % 2 == 0;
			unsigned int vRowWidth = width + (isWideRow ? 1 : 0);
			if (!isPeakRow) {
				ret->addVertex(appendixCtr);
				ret->addEdgeSym(appendixCtr++, outerVertex);
				ret->addVertex(appendixCtr);
				ret->addEdgeSym(appendixCtr++, outerVertex + vRowWidth - 1);
			}
			outerVertex += vRowWidth;
		}
		/* Create top appendices */
		for (outerVertex = 1; outerVertex < width; outerVertex += 2) {
			ret->addVertex(appendixCtr);
			ret->addEdgeSym(appendixCtr++, outerVertex);
		}
		/* Create bottom appendices */
		for (outerVertex = startingSize - width; outerVertex < startingSize; outerVertex += 2) {
			ret->addVertex(appendixCtr);
			ret->addEdgeSym(appendixCtr++, outerVertex);
		}
		return ret;
	}

	/* Same as honey combs plus, except every hexagon has an inner connection between
	 * the upper two non-peak vertices, also the lowest vertex row is connected. */
	SimpleGraph<unsigned int>* GGenerator::createHoneyCombsInner(unsigned int width, unsigned int rows) {
		auto ret = createHoneyCombs(width, rows);
		unsigned int startingSize = ret->getVertexCount();
		/* Index for newly appended vertices */
		unsigned int appendixCtr = ret->getVertexCount();
		unsigned int outerVertex = 0;
		/* Create inner connections */
		for (unsigned int vRow = 0; vRow < (rows + 1) * 2 - 1; vRow++) {
			bool isWideRow = ((vRow - 1) / 2) % 2 == 0;
			bool isPeakRow = vRow % 2 == 0;
			unsigned int vRowWidth = width + (isWideRow ? 1 : 0);
			if (!isPeakRow) {
				for (unsigned int i = 1; i < vRowWidth; i++) {
					ret->addEdgeSym(outerVertex + i - 1, outerVertex + i);
				}
			}
			outerVertex += vRowWidth;
		}
		/* Create side appendices */
		outerVertex = width;
		for (unsigned int vRow = 1; vRow < (rows + 1) * 2 - 1; vRow++) {
			bool isWideRow = ((vRow - 1) / 2) % 2 == 0;
			bool isPeakRow = vRow % 2 == 0;
			unsigned int vRowWidth = width + (isWideRow ? 1 : 0);
			if (!isPeakRow) {
				ret->addVertex(appendixCtr);
				ret->addEdgeSym(appendixCtr++, outerVertex);
				ret->addVertex(appendixCtr);
				ret->addEdgeSym(appendixCtr++, outerVertex + vRowWidth - 1);
			}
			outerVertex += vRowWidth;
		}
		/* Create top appendices */
		for (outerVertex = 1; outerVertex < width; outerVertex += 2) {
			ret->addVertex(appendixCtr);
			ret->addEdgeSym(appendixCtr++, outerVertex);
		}
		/* Create bottom appendices */
		for (outerVertex = startingSize - width; outerVertex < startingSize; outerVertex += 2) {
			ret->addVertex(appendixCtr);
			ret->addEdgeSym(appendixCtr++, outerVertex);
		}
		return ret;
	}
}