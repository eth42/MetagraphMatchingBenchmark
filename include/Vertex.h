#pragma once

namespace maxmatching {
	template<class Label>
	class Vertex;
}

#include <vector>
#include "BaseVertex.h"
#include "Statistics.h"
#include "Debug.h"
#include "CherryBlossom.h"
#include "CherryTree.h"
#include "List.h"
#include "ListElement.h"

namespace maxmatching {
	template<class Label>
	class Vertex : public BaseVertex<Label> {
	private:
		static unsigned int idCtr;
		static unsigned int nextId();

		CherryTree<Label>* containingTree;
		CherryBlossom<Label>* containingBlossom;
		Vertex<Label>* matchingPartner;
		Vertex<Label>* evenParent;
		List<Vertex<Label>>* growQueue;
		int level;
	public:
		std::vector<Vertex<Label>*> neighbors;
		ListElement<Vertex<Label>> listElemOddChildren;
		ListElement<Vertex<Label>> listElemBlossomVertices;
		ListElement<Vertex<Label>> listElemGrowQueue;
		List<Vertex<Label>> oddChildren;
		List<CherryBlossom<Label>> bearingBlossoms;

		Vertex(Label l);
		~Vertex();

		void setGrowQueue(List<Vertex>* growQueue);
		List<Vertex>* getGrowQueue();
		void enqueue();
		void dequeue();
		void updateLevel();
		int getLevel();

		void setContainingBlossom(CherryBlossom<Label>* blossom);
		CherryBlossom<Label>* getImmediateBlossom();
		CherryBlossom<Label>* getContainingBlossom();
		void setContainingTree(CherryTree<Label>* t);
		CherryTree<Label>* getContainingTree();
		void setEvenParent(Vertex<Label>* newParent);
		Vertex<Label>* getEvenParent();
		void setMatchingPartner(Vertex<Label>* newPartner);
		Vertex<Label>* getMatchingPartner();

		Vertex<Label>* getNeighbor(int i);

		bool isEven();
		bool isOdd();

		List<Vertex<Label>>* createEvenPathToRoot();
		List<Vertex<Label>>* createOddPathToRoot();
		List<Vertex<Label>>* createEvenPathToReceptacle(CherryBlossom<Label>* blossom);
		List<Vertex<Label>>* createOddPathToReceptacle(CherryBlossom<Label>* blossom);

		void reset();

		static void resetIds();
	};

	template <class Label>
	unsigned int Vertex<Label>::idCtr = 1;
	template <class Label>
	unsigned int Vertex<Label>::nextId() {
		return Vertex<Label>::idCtr++;
	}
	template <class Label>
	void Vertex<Label>::resetIds() {
		Vertex<Label>::idCtr = 1;
	}
}

#include "Vertex.tpp"