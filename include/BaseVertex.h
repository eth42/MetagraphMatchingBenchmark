#pragma once

namespace maxmatching {
	/* Base class for labeled vertices */
	template <class Label>
	class BaseVertex {
	public:
		unsigned int id;
		Label label;
		inline BaseVertex(int id, Label l) : id(id), label(l) {}
		inline virtual ~BaseVertex() {}

		inline unsigned int getId() {
			return this->id;
		}
		inline Label getLabel() {
			return this->label;
		}
	};
}