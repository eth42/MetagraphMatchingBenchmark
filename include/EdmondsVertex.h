#pragma once
#include "BaseVertex.h"

namespace maxmatching {
	/* Basically just a simple implementation of the abstract BaseVertex class. */
	template <class Label>
	class EdmondsVertex : public BaseVertex<Label> {
	private:
		static unsigned int idCtr;
		static unsigned int nextId();
	public:
		EdmondsVertex(Label l);
		~EdmondsVertex();

		static void resetIds();
	};

	template <class Label>
	unsigned int EdmondsVertex<Label>::idCtr = 0;
	template <class Label>
	unsigned int EdmondsVertex<Label>::nextId() {
		return EdmondsVertex<Label>::idCtr++;
	}
	template <class Label>
	void EdmondsVertex<Label>::resetIds() {
		EdmondsVertex<Label>::idCtr = 0;
	}
}

#include "EdmondsVertex.tpp"
