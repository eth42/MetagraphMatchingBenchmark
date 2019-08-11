#pragma once

namespace maxmatching {
namespace norm {
	template <class Label>
	class HalfEdge;
}
}

#include "norm/MVertex.h"
#include "Statistics.h"

namespace maxmatching {
namespace norm {
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

#include "norm/HalfEdge.tpp"
