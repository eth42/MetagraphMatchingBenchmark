#pragma once

namespace maxmatching {
	/* Enum to describe which solver to use */
	enum SolverType {
		MultiTrees,
		MetaGraphs,
		MetaGraphsWR,
		MetaGraphsQPT,
		EdmondsBoost
#ifdef HAS_LEMON
		, EdmondsLemon
#endif
#ifdef HAS_BLOSSOM_IV
		, BlossomIV
#endif
#ifdef HAS_BLOSSOM_V
		, BlossomV
#endif
	};

	/* Enum to desribe which graph source to use */
	enum SourceType {
		Filesystem,
		RandomBoost,
		RandomDimacs,
#ifdef HAS_FADE
		RandomTriangle,
#endif
		RandomEuclid2d,
		RandomEuclid3d,
		RandomEuclid10d,
		RandomEuclid20d,
		WorstCaseGabow,
		TriangleSeriesA,
		TriangleSeriesB,
		HoneyCombs,
		HoneyCombsPlus,
		HoneyCombsCaps,
		HoneyCombsInner
	};
}
