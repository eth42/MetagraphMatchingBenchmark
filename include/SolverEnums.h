#pragma once

namespace maxmatching {
	/* Describes which strategy to use for sorting vertices
	 * prior to the algorithms in MultiTreeSolver and MetaGraphsSolver. */
	enum PreSortStrategy {
		None,
		MinDegree,
		MaxDegree
	};
}