#pragma once

namespace maxmatching {
namespace wr {
	template <class Label>
	class HalfEdge;
}
}

#include "wr/MVertex.h"
#include "Statistics.h"

namespace maxmatching {
namespace wr {
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

#include "wr/HalfEdge.tpp"
