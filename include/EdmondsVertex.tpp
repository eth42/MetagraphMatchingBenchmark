#include "EdmondsVertex.h"

namespace maxmatching {
	template <class Label>
	EdmondsVertex<Label>::EdmondsVertex(Label l) : BaseVertex<Label>(EdmondsVertex<Label>::nextId(), l) {}

	template <class Label>
	EdmondsVertex<Label>::~EdmondsVertex() {}
}