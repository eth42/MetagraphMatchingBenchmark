#include "Vertex.h"

namespace maxmatching {
	template <class Label>
	Vertex<Label>::Vertex(Label l)
		: BaseVertex<Label>(Vertex<Label>::nextId(), l)
		, containingTree(nullptr)
		, containingBlossom(nullptr)
		, matchingPartner(nullptr)
		, evenParent(nullptr)
		, growQueue(nullptr)
		, level(0)
		, neighbors()
		, listElemOddChildren(this)
		, listElemBlossomVertices(this)
		, listElemGrowQueue(this)
		, oddChildren()
		, bearingBlossoms() {
		Statistics::incrementVertCreated();
	}

	/* Deletes the vertex and all trees and blossoms it is contained in. */
	template <class Label>
	Vertex<Label>::~Vertex() {
		DEBUG("Deleting Vertex " << this->id << "\n");
		if (this->containingTree != nullptr) {
			delete(this->containingTree);
		}
		if (this->containingBlossom != nullptr) {
			delete(this->getContainingBlossom());
		}
		while (!this->bearingBlossoms.isEmpty()) {
			delete(this->bearingBlossoms.popElem()->value);
		}
		Statistics::incrementVertDeleted();
	}


	template <class Label>
	void Vertex<Label>::setGrowQueue(List<Vertex>* growQueue) {
		this->growQueue = growQueue;
	}

	template <class Label>
	List<Vertex<Label>>* Vertex<Label>::getGrowQueue() {
		return this->growQueue;
	}

	template <class Label>
	void Vertex<Label>::enqueue() {
		if (this->listElemGrowQueue.containingList == nullptr) {
			DEBUG("Enqueueing " << this->id << "\n");
			this->growQueue->append(&this->listElemGrowQueue);
		}
	}

	template <class Label>
	void Vertex<Label>::dequeue() {
		if (this->listElemGrowQueue.containingList != nullptr) {
			DEBUG("Dequeueing " << this->id << "\n");
			this->growQueue->remove(&this->listElemGrowQueue);
		}
	}

	template <class Label>
	void Vertex<Label>::updateLevel() {
		DEBUG("Updating level at " << this->id << "\n");
		if (this->getMatchingPartner() != nullptr && this->getMatchingPartner()->getEvenParent() != nullptr) {
			this->level = this->getMatchingPartner()->getEvenParent()->getLevel() + 1;
		} else {
			this->level = 0;
		}
		for (auto el = this->bearingBlossoms.getFirstElement(); el != nullptr; el = el->nxtElem) {
			el->value->updateLevel();
		}
	}

	template <class Label>
	int Vertex<Label>::getLevel() {
		CherryBlossom<Label>* blossom = this->getContainingBlossom();
		if (blossom != nullptr) {
			return blossom->getLevel() + 1;
		} else {
			return this->level;
		}
	}


	template <class Label>
	void Vertex<Label>::setContainingBlossom(CherryBlossom<Label> * blossom) {
		this->containingBlossom = blossom;
	}

	template <class Label>
	CherryBlossom<Label>* Vertex<Label>::getImmediateBlossom() {
		return this->containingBlossom;
	}

	template <class Label>
	CherryBlossom<Label>* Vertex<Label>::getContainingBlossom() {
		if (this->containingBlossom == nullptr) {
			return nullptr;
		} else {
			return this->containingBlossom->getReference();
		}
	}

	template <class Label>
	void Vertex<Label>::setContainingTree(CherryTree<Label> * t) {
		this->containingTree = t;
	}

	template <class Label>
	CherryTree<Label>* Vertex<Label>::getContainingTree() {
		return this->containingTree;
	}

	/* Sets the even parent of this vertex. Also updates the oddChildren
	* list in the old and new parent as well as the level of the matching partner.
	* If the matching partner becomes even by this action, it is also added to the grow queue. */
	template <class Label>
	void Vertex<Label>::setEvenParent(Vertex<Label> * newParent) {
		DEBUG("Even parent of " << this->id << " = " << (newParent != nullptr ? newParent->id : 0) << "\n");
		if (newParent == this->evenParent) {
			this->getMatchingPartner()->updateLevel();
			return;
		}
		bool partnerWasEven = this->matchingPartner->isEven();
		//bool parentWasNull = this->evenParent == nullptr;
		if (this->evenParent != nullptr) {
			this->evenParent->oddChildren.remove(&this->listElemOddChildren);
		}
		this->evenParent = newParent;
		if (newParent != nullptr) {
			this->evenParent->oddChildren.append(&this->listElemOddChildren);
			this->getMatchingPartner()->updateLevel();
			if (!partnerWasEven && this->matchingPartner->isEven()) {
				this->matchingPartner->enqueue();
			}
		}
	}

	template <class Label>
	Vertex<Label>* Vertex<Label>::getEvenParent() {
		return this->evenParent;
	}

	template <class Label>
	void Vertex<Label>::setMatchingPartner(Vertex<Label> * newPartner) {
		this->matchingPartner = newPartner;
	}

	template <class Label>
	Vertex<Label>* Vertex<Label>::getMatchingPartner() {
		return this->matchingPartner;
	}

	template <class Label>
	Vertex<Label>* Vertex<Label>::getNeighbor(int i) {
		return this->neighbors[i];
	}

	template <class Label>
	bool Vertex<Label>::isEven() {
		return this->containingTree != nullptr
			&& (this->matchingPartner == nullptr
				|| (this->matchingPartner->evenParent != nullptr
					&& this->matchingPartner->evenParent->containingTree == this->containingTree));
	}

	template <class Label>
	bool Vertex<Label>::isOdd() {
		return this->containingTree != nullptr
			&& this->evenParent != nullptr
			&& this->evenParent->containingTree == this->containingTree;
	}

	template <class Label>
	List<Vertex<Label>>* Vertex<Label>::createEvenPathToRoot() {
		List<Vertex<Label>>* ret = new List<Vertex>();
		ret->push(this);
		Vertex* v = this->matchingPartner;
		while (v != nullptr) {
			ret->append(v);
			ret->append(v->evenParent);
			v = v->evenParent->matchingPartner;
		}
		return ret;
	}
	template <class Label>
	List<Vertex<Label>>* Vertex<Label>::createOddPathToRoot() {
		List<Vertex<Label>>* ret = this->evenParent->createEvenPathToRoot();
		ret->push(this);
		return ret;
	}

	template <class Label>
	List<Vertex<Label>>* Vertex<Label>::createEvenPathToReceptacle(CherryBlossom<Label> * blossom) {
		List<Vertex<Label>>* ret = new List<Vertex<Label>>();
		Vertex* v = this;
		while (v != blossom->getReceptacle()) {
			ret->append(v);
			ret->append(v->getMatchingPartner());
			v = v->getMatchingPartner()->getEvenParent();
		}
		ret->append(blossom->getReceptacle());
		return ret;
	}
	template <class Label>
	List<Vertex<Label>>* Vertex<Label>::createOddPathToReceptacle(CherryBlossom<Label> * blossom) {
		List<Vertex<Label>>* ret = this->getMatchingPartner()->createEvenPathToReceptacle(blossom);
		ret->push(this);
		return ret;
	}

	/* Frees the memory allocated by the adjacency list. */
	template <class Label>
	void Vertex<Label>::reset() {
		DEBUG("Resetting " << this->id << "\n");
		/* Clear rooted structures */
		CherryTree<Label> * tree = this->getContainingTree();
		if (tree != nullptr) {
			DEBUG("Pre delete tree\n");
			delete(tree);
			DEBUG("Post delete tree\n");
		}
		DEBUG("Pre deleting blossoms\n");
		while (!this->bearingBlossoms.isEmpty()) {
			CherryBlossom<Label>* blossom = this->bearingBlossoms.popElem()->value;
			delete(blossom);
		}
		DEBUG("Post deleting blossoms\n");
		/* Remove from queue */
		DEBUG("Pre dequeue\n");
		this->dequeue();
		DEBUG("Post dequeue\n");
		/* Reset variables */
		DEBUG("Pre remainder\n");
		if (this->evenParent != nullptr) {
			this->evenParent->oddChildren.remove(&this->listElemOddChildren);
		}
		this->evenParent = nullptr;
		this->setMatchingPartner(nullptr);
		DEBUG("Pre remainder\n");
		this->level = 0;
		DEBUG("Done resetting " << this->id << "\n");
	}
}
