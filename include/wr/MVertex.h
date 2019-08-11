#pragma once

namespace maxmatching {
namespace wr {
	template<class Label>
	class MVertex;
}
}

#include <vector>
#include <functional>
#include "BaseVertex.h"
#include "Debug.h"
#include "Statistics.h"
#include "List.h"
#include "ListElement.h"
#include "wr/HalfEdge.h"
#include "wr/MCherryBlossom.h"
#include "wr/MCherryTree.h"

namespace maxmatching {
namespace wr {
	template<class Label>
	class MVertex : public BaseVertex<Label> {
	private:
		/* Used to enumerate the vertices */
		static unsigned int idCtr;
		static unsigned int nextId();

		/* The represented tree of this metavertex. If it's not a metavertex, this is nullptr. */
		MCherryTree<Label>* representedTree;
		/* The tree currently containing this vertex. */
		MCherryTree<Label>* containingTree;
		/* The blossom, whose corolla currently contains this vertex.
		 * This is not necessarly the representant in the disjoint-set data structure!
		 * To access that, use the getContainingBlossom() method. */
		MCherryBlossom<Label>* containingBlossom;
		/* HalfEdge directed to the matching partner of this vertex. */
		HalfEdge<Label>* matchingPartner;
		/* HalfEdge directed to the even parent of this vertex. */
		HalfEdge<Label>* evenParent;
		/* Reference to the GrowQueue of the solver using this vertex, so the vertex
		 * can enqueue and dequeue itself. */
		List<MVertex>* growQueue;
		/* Same as GrowQueue reference except for waiting room */
		List<MVertex>* waitingRoom;
		/* Level of the vertex. */
		int level;
	public:
		/* Describes how the label of metavertices are to be computed. */
		static std::function<Label(MCherryTree<Label>*)> accumulator;

		/* Adjacency list */
		std::vector<HalfEdge<Label>*> neighbors;
		/* ListElement for the oddChildren list in the MVertex class
		 * Necessary for O(1) deletion */
		ListElement<HalfEdge<Label>> listElemOddChildren;
		/* ListElement for the vertices list in the MCherryBlossom class
		 * Necessary for O(1) deletion */
		ListElement<MVertex> listElemBlossomVertices;
		/* ListElement for the GrowQueue in the MetaGraphsSolver class
		 * Necessary for O(1) deletion */
		ListElement<MVertex> listElemGrowQueue;
		/* List of all odd children of this vertex */
		List<HalfEdge<Label>> oddChildren;
		/* List of all cherry blossoms, for which this vertex is the receptacle */
		List<MCherryBlossom<Label>> bearingBlossoms;

		MVertex(Label l);
		MVertex(MCherryTree<Label>* representedTree);
		~MVertex();

		void setGrowQueue(List<MVertex<Label>>* growQueue);
		List<MVertex>* getGrowQueue();
		void setWaitingRoom(List<MVertex<Label>>* waitingRoom);
		List<MVertex>* getWaitingRoom();
		void enqueue();
		void dequeue();
		void startWaiting();
		void stopWaiting();
		void updateLevel();
		int getLevel();

		MCherryTree<Label>* getRepresentedTree();
		void setContainingBlossom(MCherryBlossom<Label>* blossom);
		MCherryBlossom<Label>* getImmediateBlossom();
		MCherryBlossom<Label>* getContainingBlossom();
		void setContainingTree(MCherryTree<Label>* t);
		MCherryTree<Label>* getContainingTree();
		void setEvenParent(HalfEdge<Label>* newParent);
		MVertex<Label>* getEvenParent();
		HalfEdge<Label>* getEvenParentEdge();
		void setMatchingPartner(HalfEdge<Label>* newPartner);
		MVertex<Label>* getMatchingPartner();
		HalfEdge<Label>* getMatchingPartnerEdge();

		MVertex<Label>* getNeighbor(int i);

		bool isEven();
		bool isOdd();

		List<HalfEdge<Label>>* createEvenPathToRoot();
		List<HalfEdge<Label>>* createOddPathToRoot();
		List<HalfEdge<Label>>* createEvenPathToReceptacle(MCherryBlossom<Label>* blossom);
		List<HalfEdge<Label>>* createOddPathToReceptacle(MCherryBlossom<Label>* blossom);

		bool hasNeighbor(MVertex<Label>* v);
		void reset();
		void clearEdges();

		static void resetIds();
	};

	template <class Label>
	unsigned int MVertex<Label>::idCtr = 0;
	template <class Label>
	unsigned int MVertex<Label>::nextId() {
		return MVertex<Label>::idCtr++;
	}
	template <class Label>
	void MVertex<Label>::resetIds() {
		MVertex<Label>::idCtr = 0;
	}

}
}
#include "wr/MVertex.tpp"
