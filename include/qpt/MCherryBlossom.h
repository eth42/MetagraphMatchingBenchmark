#pragma once

namespace maxmatching {
namespace qpt {
	template <class Label>
	class MCherryBlossom;
}
}

#include <functional>
#include "Debug.h"
#include "Statistics.h"
#include "List.h"
#include "ListElement.h"
#include "qpt/MVertex.h"

namespace maxmatching {
namespace qpt {
	template <class Label>
	class MCherryBlossom {
	private:
		/* Receptacle of the cherry blossom */
		MVertex<Label>* receptacle;
		/* List of vertices in the corolla */
		List<MVertex<Label>> vertices;
		/* Parent in the disjoint-set data structure */
		MCherryBlossom<Label>* parentBlossom;
		/* Complexity if this blossom */
		int complexity;
		/* Level of this blossom */
		int level;
	public:
		/* List of children in the disjoint-set data structure */
		List<MCherryBlossom<Label>> childBlossoms;
		/* ListElement for the children lists in the disjoint-set data structure
		 * Necessary for O(1) deletion */
		ListElement<MCherryBlossom<Label>> childElement;
		/* ListElement for the bearingBlossoms list in the MVertex class
		 * Necessary for O(1) deletion */
		ListElement<MCherryBlossom<Label>> listElement;

		explicit MCherryBlossom(MVertex<Label>* receptacle);
		~MCherryBlossom();

		void add(MVertex<Label>* v);
		void remove(MVertex<Label>* v);
		void rotate(MVertex<Label>* newReceptacle);
		void setReceptacle(MVertex<Label>* newReceptacle);
		MVertex<Label>* getReceptacle();
		void foreachVertex(const std::function<void(MVertex<Label>*)>& fun);
		void merge(MCherryBlossom<Label>* child);
		void remove(MCherryBlossom<Label>* child);
		MCherryBlossom<Label>* getReference();

		void updateLevel();
		int getLevel();
		int getComplexity();
	};
}
}

#include "qpt/MCherryBlossom.tpp"
