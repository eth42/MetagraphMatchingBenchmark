#pragma once

namespace maxmatching {
namespace norm {
	template <class Label>
	class MCherryBlossom;
}
}

#include <functional>
#include "Debug.h"
#include "Statistics.h"
#include "List.h"
#include "ListElement.h"
#include "norm/MVertex.h"

namespace maxmatching {
namespace norm {
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
		unsigned int complexity;
		/* Level of this blossom */
		unsigned int level;
	public:
		/* List of children in the disjoint-set data structure */
		List<MCherryBlossom<Label>> childBlossoms;
		/* ListElement for the children lists in the disjoint-set data structure
		 * Necessary for O(1) deletion */
		ListElement<MCherryBlossom<Label>> childElement;
		/* ListElement for the bearingBlossoms list in the MVertex class
		 * Necessary for O(1) deletion */
		ListElement<MCherryBlossom<Label>> listElement;

		MCherryBlossom(MVertex<Label>* receptacle);
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
		unsigned int getLevel();
		unsigned int getComplexity();
	};

}
}
#include "norm/MCherryBlossom.tpp"
