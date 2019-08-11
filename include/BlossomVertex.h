#pragma once
#include "BaseVertex.h"

namespace maxmatching {
	/* Basically just a simple implementation of the abstract BaseVertex class. */
	template <class Label>
	class BlossomVertex : public BaseVertex<Label> {
	private:
		static unsigned int idCtr;
		static unsigned int nextId();
	public:
		BlossomVertex* mirror;

		BlossomVertex(Label l);
		~BlossomVertex();

		static void resetIds();
	};

	template <class Label>
	unsigned int BlossomVertex<Label>::idCtr = 0;
	template <class Label>
	unsigned int BlossomVertex<Label>::nextId() {
		return BlossomVertex<Label>::idCtr++;
	}
	template <class Label>
	void BlossomVertex<Label>::resetIds() {
		BlossomVertex<Label>::idCtr = 0;
	}
}

#include "BlossomVertex.tpp"
