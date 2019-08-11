#pragma once

namespace maxmatching {
namespace qpt {
	template <class Label>
	class HalfEdge;
}
}

#include "qpt/MVertex.h"
#include "Statistics.h"

namespace maxmatching {
namespace qpt {
	/* Basic half edge implementation. Basicly a struct, if you will. */
	template <class Label>
	class HalfEdge {
	public:
		MVertex<Label>* start;
		MVertex<Label>* end;
		HalfEdge<Label>* label;
		HalfEdge<Label>* inverse;

		HalfEdge(MVertex<Label>* start, MVertex<Label>* end, HalfEdge<Label>* label);
		~HalfEdge();
	};
}
}

#include "qpt/HalfEdge.tpp"
