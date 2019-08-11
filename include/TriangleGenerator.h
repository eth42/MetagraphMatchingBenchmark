#pragma once
#include "SimpleGraph.h"
#ifdef HAS_FADE
#include <Fade_2D.h>
#endif
#include <vector>
#include <map>

namespace maxmatching {
	class TriangleGenerator {
	private:
		typedef std::map<double, unsigned int> yMap;
		typedef std::map<double, yMap> xMap;

		template <typename K, typename V>
		bool mapHasKey(std::map<K, V> &m, K &k) {
			return m.count(k) > 0;
		}
		template <typename K, typename V>
		V& mapGet(std::map<K, V> &m, K &k, V &fallback) {
			auto it = m.find(k);
			if (it != m.end()) {
				return (*it).second;
			}
			return fallback;
		}

		inline yMap& xMapGet(xMap &m, double &k, yMap &y) {
			return mapGet<double, yMap>(m,k,y);
		}
		inline unsigned int& yMapGet(yMap &m, double &k, unsigned int &f) {
			return mapGet<double, unsigned int>(m,k,f);
		}
		
	public:
		TriangleGenerator();
		~TriangleGenerator();

		SimpleGraph<unsigned int>* generate(unsigned int nPoints, std::mt19937_64 &rgen);
	};

}
