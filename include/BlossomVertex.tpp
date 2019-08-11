#include "BlossomVertex.h"

namespace maxmatching {
	template <class Label>
	BlossomVertex<Label>::BlossomVertex(Label l) : BaseVertex<Label>(BlossomVertex<Label>::nextId(), l), mirror(nullptr) {}

	template <class Label>
	BlossomVertex<Label>::~BlossomVertex() {}
}