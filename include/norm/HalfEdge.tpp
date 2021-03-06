#include "norm/HalfEdge.h"

namespace maxmatching {
namespace norm {
	template <class Label>
	HalfEdge<Label>::HalfEdge(MVertex<Label>* start, MVertex<Label>* end, HalfEdge<Label>* label)
		: start(start)
		, end(end)
		, label(label)
		, inverse(nullptr) {
		Statistics::incrementEdgeCreated();
	}


	template <class Label>
	HalfEdge<Label>::~HalfEdge() {
		Statistics::incrementEdgeDeleted();
	}
}
}
