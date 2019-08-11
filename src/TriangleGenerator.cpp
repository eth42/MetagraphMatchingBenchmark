#include "TriangleGenerator.h"

namespace maxmatching {
	TriangleGenerator::TriangleGenerator() {}


	TriangleGenerator::~TriangleGenerator() {}

	SimpleGraph<unsigned int>* TriangleGenerator::generate(unsigned int nPoints, std::mt19937_64& rgen) {
		SimpleGraph<unsigned int>* ret = new SimpleGraph<unsigned int>();
#ifdef HAS_FADE
		yMap dummyMap;
		unsigned int dummyIdx = 0;
		GEOM_FADE2D::Fade_2D delaunay;
		std::vector<GEOM_FADE2D::Point2> points;
		xMap idxMap;
		for (unsigned int i = 0; i < nPoints; i++) {
			/* Probably change the range here? */
			GEOM_FADE2D::Point2 newP = GEOM_FADE2D::Point2(rgen(), rgen());
			double newPx = newP.x();
			double newPy = newP.y();
			auto insertX = idxMap.insert(std::make_pair(newPx, yMap()));
			auto insertY = insertX.first->second.insert(std::make_pair(newPy, i));
			if (insertY.second == false) {
				i--;
			} else {
				points.push_back(newP);
				ret->addVertex(i);
			}
		}
		delaunay.insert(points);
		std::vector<GEOM_FADE2D::Triangle2*> trianglePointers;
		delaunay.getTrianglePointers(trianglePointers);
		for (GEOM_FADE2D::Triangle2* tp : trianglePointers) {
			for (unsigned int i = 0; i < 3; i++) {
				GEOM_FADE2D::Point2* p1 = tp->getCorner(i);
				GEOM_FADE2D::Point2* p2 = tp->getCorner((i + 1) % 3);
				double p1x = p1->x();
				double p1y = p1->y();
				double p2x = p2->x();
				double p2y = p2->y();
				yMap& p1ym = xMapGet(idxMap, p1x, dummyMap);
				yMap& p2ym = xMapGet(idxMap, p2x, dummyMap);
				unsigned int ip1 = yMapGet(p1ym, p1y, dummyIdx);
				unsigned int ip2 = yMapGet(p2ym, p2y, dummyIdx);
				ret->addEdgeSymSafe(ip1, ip2);
			}
		}
#else
		(void)nPoints;
		(void)rgen;
#endif
		return ret;
	}
}
