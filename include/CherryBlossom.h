#pragma once

namespace maxmatching {
	template <class Label>
	class CherryBlossom;
}

#include <functional>
#include "Statistics.h"
#include "Debug.h"
#include "List.h"
#include "ListElement.h"
#include "Vertex.h"

namespace maxmatching {
	template <class Label>
	class CherryBlossom {
	private:
		/* Receptacle of the cherry blossom */
		Vertex<Label>* receptacle;
		/* List of vertices in the corolla */
		List<Vertex<Label>> vertices;
		/* Parent in the disjoint-set data structure */
		CherryBlossom<Label>* parentBlossom;
		/* Complexity if this blossom */
		int complexity;
		/* Level of this blossom */
		int level;
	public:
		/* List of children in the disjoint-set data structure */
		List<CherryBlossom<Label>> childBlossoms;
		/* ListElement for the children lists in the disjoint-set data structure
		 * Necessary for O(1) deletion */
		ListElement<CherryBlossom<Label>> childElement;
		/* ListElement for the bearingBlossoms list in the Vertex class
		 * Necessary for O(1) deletion */
		ListElement<CherryBlossom<Label>> listElement;

		CherryBlossom(Vertex<Label>* receptacle);
		~CherryBlossom();

		void add(Vertex<Label>* v);
		void remove(Vertex<Label>* v);
		void rotate(Vertex<Label>* newReceptacle);
		void setReceptacle(Vertex<Label>* newReceptacle);
		Vertex<Label>* getReceptacle();
		void foreachVertex(std::function<void(Vertex<Label>*)> fun);
		void merge(CherryBlossom<Label>* child);
		void remove(CherryBlossom<Label>* child);
		CherryBlossom<Label>* getReference();

		void updateLevel();
		int getLevel();
		int getComplexity();
	};
}

#include "CherryBlossom.tpp"