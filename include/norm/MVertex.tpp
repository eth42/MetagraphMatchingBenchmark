#include "norm/MVertex.h"

namespace maxmatching {
namespace norm {
	template <class Label>
	std::function<Label(MCherryTree<Label>*)> MVertex<Label>::accumulator = [](MCherryTree<Label> * t) -> Label {return t->root->getLabel(); };

	template <class Label>
	MVertex<Label>::MVertex(Label l)
		: BaseVertex<Label>(MVertex<Label>::nextId(), l)
		, representedTree(nullptr)
		, containingTree(nullptr)
		, containingBlossom(nullptr)
		, matchingPartner(nullptr)
		, evenParent(nullptr)
		, growQueue(nullptr)
		, level(0)
		, neighbors()
		, listElemOddChildren(nullptr)
		, listElemBlossomVertices(this)
		, listElemGrowQueue(this)
		, oddChildren()
		, bearingBlossoms() {
		Statistics::incrementVertCreated();
	}

	template <class Label>
	MVertex<Label>::MVertex(MCherryTree<Label>* representedTree)
		: BaseVertex<Label>(MVertex<Label>::nextId(), MVertex<Label>::accumulator(representedTree))
		, representedTree(representedTree)
		, containingTree(nullptr)
		, containingBlossom(nullptr)
		, matchingPartner(nullptr)
		, evenParent(nullptr)
		, growQueue(nullptr)
		, level(0)
		, neighbors()
		, listElemOddChildren(nullptr)
		, listElemBlossomVertices(this)
		, listElemGrowQueue(this)
		, oddChildren()
		, bearingBlossoms() {
		DEBUG("Created MVertex " << this->id << "\n");
		Statistics::incrementVertCreated();
	}

	/* Deletes the vertex and all trees and blossoms it is contained in. */
	template <class Label>
	MVertex<Label>::~MVertex() {
		DEBUG("Deleting MVertex " << this->id << "\n");
		if (this->containingTree != nullptr) {
			delete(this->containingTree);
		}
		if (this->containingBlossom != nullptr) {
			delete(this->getContainingBlossom());
		}
		while (!this->bearingBlossoms.isEmpty()) {
			delete(this->bearingBlossoms.popElem()->value);
		}
		this->clearEdges();
		Statistics::incrementVertDeleted();
	}


	template <class Label>
	void MVertex<Label>::setGrowQueue(List<MVertex<Label>> * growQueue) {
		this->growQueue = growQueue;
	}

	template <class Label>
	List<MVertex<Label>>* MVertex<Label>::getGrowQueue() {
		return this->growQueue;
	}

	template <class Label>
	void MVertex<Label>::enqueue() {
		if (this->listElemGrowQueue.containingList == nullptr) {
			DEBUG("Enqueueing " << this->id << "\n");
			this->growQueue->append(&this->listElemGrowQueue);
		}
	}

	template <class Label>
	void MVertex<Label>::dequeue() {
		if (this->listElemGrowQueue.containingList != nullptr) {
			DEBUG("Dequeueing " << this->id << "\n");
			this->growQueue->remove(&this->listElemGrowQueue);
		}
	}

	template <class Label>
	void MVertex<Label>::updateLevel() {
		if (this->getMatchingPartner() != nullptr && this->getMatchingPartner()->getEvenParent() != nullptr) {
			this->level = this->getMatchingPartner()->getEvenParent()->getLevel() + 1;
		} else {
			this->level = 0;
		}
		DEBUG("Updating level at " << this->id << " to " << this->getLevel() << "\n");
		for (auto el = this->bearingBlossoms.getFirstElement(); el != nullptr; el = el->nxtElem) {
			el->value->updateLevel();
		}
	}

	template <class Label>
	unsigned int MVertex<Label>::getLevel() {
		MCherryBlossom<Label>* blossom = this->getContainingBlossom();
		if (blossom != nullptr) {
			return blossom->getLevel() + 1;
		} else {
			return this->level;
		}
	}

	template <class Label>
	MCherryTree<Label>* MVertex<Label>::getRepresentedTree() {
		return this->representedTree;
	}

	template <class Label>
	void MVertex<Label>::setContainingBlossom(MCherryBlossom<Label> * blossom) {
		this->containingBlossom = blossom;
	}

	template <class Label>
	MCherryBlossom<Label>* MVertex<Label>::getImmediateBlossom() {
		return this->containingBlossom;
	}

	template <class Label>
	MCherryBlossom<Label>* MVertex<Label>::getContainingBlossom() {
		if (this->containingBlossom == nullptr) {
			return nullptr;
		} else {
			return this->containingBlossom->getReference();
		}
	}

	template <class Label>
	void MVertex<Label>::setContainingTree(MCherryTree<Label> * t) {
		this->containingTree = t;
	}

	template <class Label>
	MCherryTree<Label>* MVertex<Label>::getContainingTree() {
		return this->containingTree;
	}

	/* Sets the even parent of this vertex. Also updates the oddChildren
	 * list in the old and new parent as well as the level of the matching partner.
	 * If the matching partner becomes even by this action, it is also added to the grow queue. */
	template <class Label>
	void MVertex<Label>::setEvenParent(HalfEdge<Label> * newParent) {
		DEBUG("@" << this->id
			<< ": Even parent of "
			<< (newParent != nullptr ? newParent->start->id : this->id)
			<< " = "
			<< (newParent != nullptr ? newParent->end->id : 0)
			<< "\n");
		if (newParent == this->evenParent) {
			this->getMatchingPartner()->updateLevel();
			return;
		}
		bool partnerWasEven = this->getMatchingPartner()->isEven();
		//bool parentWasNull = this->getEvenParent() == nullptr;
		if (this->getEvenParent() != nullptr) {
			this->getEvenParent()->oddChildren.remove(&this->listElemOddChildren);
		}
		this->evenParent = newParent;
		if (newParent != nullptr) {
			this->getEvenParent()->oddChildren.append(&this->listElemOddChildren);
			this->listElemOddChildren.value = newParent->inverse;
			this->getMatchingPartner()->updateLevel();
			if (!partnerWasEven && this->getMatchingPartner()->isEven()) {
				this->getMatchingPartner()->enqueue();
			}
		}
	}

	template <class Label>
	MVertex<Label>* MVertex<Label>::getEvenParent() {
		if (this->evenParent == nullptr) {
			return nullptr;
		} else {
			return this->evenParent->end;
		}
	}

	template <class Label>
	HalfEdge<Label>* MVertex<Label>::getEvenParentEdge() {
		return this->evenParent;
	}

	template <class Label>
	void MVertex<Label>::setMatchingPartner(HalfEdge<Label> * newPartner) {
		DEBUG("@" << this->id
			<< ": Matching partner of "
			<< (newPartner != nullptr ? newPartner->start->id : this->id)
			<< " = " <<
			(newPartner != nullptr ? newPartner->end->id : 0)
			<< "\n");
		this->matchingPartner = newPartner;
	}

	template <class Label>
	MVertex<Label> * MVertex<Label>::getMatchingPartner() {
		if (this->matchingPartner == nullptr) {
			return nullptr;
		} else {
			return this->matchingPartner->end;
		}
	}

	template <class Label>
	HalfEdge<Label>* MVertex<Label>::getMatchingPartnerEdge() {
		return this->matchingPartner;
	}

	template <class Label>
	MVertex<Label>* MVertex<Label>::getNeighbor(unsigned int i) {
		return this->neighbors[i]->end;
	}


	template <class Label>
	bool MVertex<Label>::isEven() {
		return this->containingTree != nullptr
			&& (this->getMatchingPartner() == nullptr
				|| (this->getMatchingPartner()->getEvenParent() != nullptr
					&& this->getMatchingPartner()->getEvenParent()->containingTree == this->containingTree));
	}

	template <class Label>
	bool MVertex<Label>::isOdd() {
		return this->containingTree != nullptr
			&& this->getEvenParent() != nullptr
			&& this->getEvenParent()->containingTree == this->containingTree;
	}

	template <class Label>
	List<HalfEdge<Label>>* MVertex<Label>::createEvenPathToRoot() {
		List<HalfEdge<Label>>* ret = new List<HalfEdge<Label>>();
		HalfEdge<Label>* e = this->getMatchingPartnerEdge();
		while (e != nullptr) {
			HalfEdge<Label>* pe = e->end->getEvenParentEdge();
			ret->append(e);
			ret->append(pe);
			e = pe->end->getMatchingPartnerEdge();
		}
		return ret;
	}
	template <class Label>
	List<HalfEdge<Label>>* MVertex<Label>::createOddPathToRoot() {
		List<HalfEdge<Label>>* ret = this->getEvenParent()->createEvenPathToRoot();
		ret->push(this->getEvenParentEdge());
		return ret;
	}

	template <class Label>
	List<HalfEdge<Label>>* MVertex<Label>::createEvenPathToReceptacle(MCherryBlossom<Label> * blossom) {
		List<HalfEdge<Label>>* ret = new List<HalfEdge<Label>>();
		if (this == blossom->getReceptacle()) {
			return ret;
		}
		HalfEdge<Label>* me = this->getMatchingPartnerEdge();
		ret->append(me);
		HalfEdge<Label>* e = me->end->getEvenParentEdge();
		while (e->end != blossom->getReceptacle()) {
			me = e->end->getMatchingPartnerEdge();
			ret->append(e);
			ret->append(me);
			e = me->end->getEvenParentEdge();
		}
		ret->append(e);
		return ret;
	}
	template <class Label>
	List<HalfEdge<Label>>* MVertex<Label>::createOddPathToReceptacle(MCherryBlossom<Label> * blossom) {
		List<HalfEdge<Label>>* ret = this->getMatchingPartner()->createEvenPathToReceptacle(blossom);
		ret->push(this->getMatchingPartnerEdge());
		return ret;
	}

	template <class Label>
	bool MVertex<Label>::hasNeighbor(MVertex<Label> * v) {
		for (auto it = this->neighbors.begin(); it != this->neighbors.end(); it++) {
			auto e = *it;
			if (e->end == v) {
				return true;
			}
		}
		return false;
	}
	template <class Label>
	void MVertex<Label>::reset() {
		DEBUG("Resetting " << this->id << "\n");
		/* Clear rooted structures */
		MCherryTree<Label> * tree = this->getContainingTree();
		if (tree != nullptr) {
			DEBUG("Pre delete tree\n");
			delete(tree);
			DEBUG("Post delete tree\n");
		}
		DEBUG("Pre deleting blossoms\n");
		while (!this->bearingBlossoms.isEmpty()) {
			MCherryBlossom<Label>* blossom = this->bearingBlossoms.popElem()->value;
			delete(blossom);
		}
		DEBUG("Post deleting blossoms\n");
		/* Remove from queue */
		DEBUG("Pre dequeue\n");
		this->dequeue();
		DEBUG("Post dequeue\n");
		/* Reset variables */
		DEBUG("Pre remainder\n");
		if (this->getEvenParent() != nullptr) {
			this->getEvenParent()->oddChildren.remove(&this->listElemOddChildren);
		}
		this->evenParent = nullptr;
		this->setMatchingPartner(nullptr);
		DEBUG("Pre remainder\n");
		this->level = 0;
		DEBUG("Done resetting " << this->id << "\n");
	}

	/* Frees the memory allocated by the adjacency list and the edges. */
	template <class Label>
	void MVertex<Label>::clearEdges() {
		for (auto it = this->neighbors.begin(); it != this->neighbors.end(); it++) {
			HalfEdge<Label>* e = *it;
			delete(e);
		}
		this->neighbors.clear();
		this->neighbors.shrink_to_fit();
		this->matchingPartner = nullptr;
		this->evenParent = nullptr;
	}
}
}
