#include "CherryBlossom.h"

namespace maxmatching {

	template <class Label>
	CherryBlossom<Label>::CherryBlossom(Vertex<Label>* receptacle)
		: receptacle(receptacle)
		, vertices()
		, parentBlossom(nullptr)
		, complexity(1)
		, level(receptacle->getLevel())
		, childBlossoms()
		, childElement(this)
		, listElement(this) {
		receptacle->bearingBlossoms.append(&this->listElement);
		Statistics::incrementBlosCreated();
	}


	template <class Label>
	CherryBlossom<Label>::~CherryBlossom() {
		DEBUG("Deleting blossom " << (this->receptacle == nullptr ? 0 : this->receptacle->id) << "\n");
		/* Clear all children blossoms */
		while (!this->childBlossoms.isEmpty()) {
			CherryBlossom<Label>* child = this->childBlossoms.getFirstElement()->value;
			this->remove(child);
			delete(child);
		}
		/* Remove from parent if necessary */
		if (this->childElement.containingList != nullptr) {
			this->parentBlossom->remove(this);
		}
		/* Remove as born blossom from receptacle */
		setReceptacle(nullptr);
		/* Remove all vertices from this blossom */
		while (!this->vertices.isEmpty()) {
			Vertex<Label>* v = this->vertices.popElem()->value;
			v->setContainingBlossom(nullptr);
		}
		Statistics::incrementBlosDeleted();
	}

	/* Sets the receptacle.
	 * Also manages the bearingBlossoms lists of the old and new receptacle. */
	template <class Label>
	void CherryBlossom<Label>::setReceptacle(Vertex<Label>* newReceptacle) {
		if (this->receptacle == newReceptacle) {
			return;
		}
		if (this->receptacle != nullptr) {
			this->receptacle->bearingBlossoms.remove(&this->listElement);
		}
		this->receptacle = newReceptacle;
		if (newReceptacle != nullptr) {
			this->level = newReceptacle->getLevel();
			newReceptacle->bearingBlossoms.append(&this->listElement);
		}
	}

	/* Returns the receptacle of the representant in the disjoint-set data structure */
	template <class Label>
	Vertex<Label>* CherryBlossom<Label>::getReceptacle() {
		if (this->parentBlossom == nullptr) {
			return this->receptacle;
		} else {
			return this->getReference()->getReceptacle();
		}
	}

	/* Adds a vertex to the cherry blossoms corolla.
	 * Removes the vertex from any other corolla containing this vertex. */
	template <class Label>
	void CherryBlossom<Label>::add(Vertex<Label>* v) {
		if (v->getImmediateBlossom() != nullptr) {
			v->getImmediateBlossom()->remove(v);
		}
		this->vertices.append(&v->listElemBlossomVertices);
		v->setContainingBlossom(this);
	}

	/* Removes a vertex from the blossom */
	template <class Label>
	void CherryBlossom<Label>::remove(Vertex<Label>* v) {
		/* Since we're not necessarily flattening blossom structures,
		 * we need to remove the vertex indirectly */
		if (v->getContainingBlossom() == this->getReference()) {
			v->listElemBlossomVertices.containingList->remove(&v->listElemBlossomVertices);
			v->setContainingBlossom(nullptr);
		}
	}

	/* Rotates the blossom according to the pseudocode. */
	template <class Label>
	void CherryBlossom<Label>::rotate(Vertex<Label>* newReceptacle) {
		if (newReceptacle->getContainingBlossom() != this) {
			return;
		}
		DEBUG("Rotating blossom " << this->receptacle->id << " to " << newReceptacle->id << "\n");
		/* The old receptacle has to be added to the corolla.
		 * This has to be done in CherryTree::rotate! */
		List<Vertex<Label>> * evenPath = newReceptacle->createEvenPathToReceptacle(this);
		List<Vertex<Label>> * oddPath = newReceptacle->createOddPathToReceptacle(this);
		auto evenEndElem = evenPath->getLastElement()->prvElem;
		auto oddEndElem = oddPath->getLastElement()->prvElem;
		if (evenEndElem->value == oddEndElem->value) {
			while (evenEndElem->prvElem->value == oddEndElem->prvElem->value) {
				evenEndElem = evenEndElem->prvElem;
				oddEndElem = oddEndElem->prvElem;
			}
			Vertex<Label>* intermediateRotate = evenEndElem->value;
			DEBUG("Intermediate ");
			evenPath->deleteStructure();
			oddPath->deleteStructure();
			this->rotate(intermediateRotate);
			this->rotate(newReceptacle);
			return;
		}
		this->receptacle->bearingBlossoms.remove(&this->listElement);
		this->remove(newReceptacle);
		Vertex<Label>* u = evenPath->pop();
		while (!evenPath->isEmpty()) {
			Vertex<Label>* s = evenPath->pop();
			Vertex<Label>* t = evenPath->pop();
			u->setEvenParent(s);
			s->setEvenParent(u);
			s->setMatchingPartner(t);
			t->setMatchingPartner(s);
			u = t;
		}
		evenPath->deleteStructure();
		u = oddPath->pop();
		/* Pop once extra, since in pseudo code, we started this path
		 * at the even parent of newReceptacle. */
		u = oddPath->pop();
		u->setEvenParent(newReceptacle);
		while (!oddPath->isEmpty()) {
			Vertex<Label>* s = oddPath->pop();
			Vertex<Label>* t = oddPath->pop();
			if (s->getMatchingPartner() != t) {
				s->setEvenParent(t);
				t->setEvenParent(s);
			}
			u = t;
		}
		oddPath->deleteStructure();
		newReceptacle->setEvenParent(nullptr);
		newReceptacle->setMatchingPartner(nullptr);
		this->add(this->receptacle);
		this->receptacle = newReceptacle;
		newReceptacle->bearingBlossoms.append(&this->listElement);
	}

	/* Helper method to iterate over all vertices */
	template <class Label>
	void CherryBlossom<Label>::foreachVertex(std::function<void(Vertex<Label>*)> fun) {
		for (auto el = this->vertices.getFirstElement(); el != nullptr; el = el->nxtElem) {
			fun(el->value);
		}
		for (auto el = this->childBlossoms.getFirstElement(); el != nullptr; el = el->nxtElem) {
			el->value->foreachVertex(fun);
		}
	}

	/* Basically the "join"/"union" in the disjoint-set data structure */
	template <class Label>
	void CherryBlossom<Label>::merge(CherryBlossom<Label>* child) {
		DEBUG("Invoking blossom merge between "
			<< (this->getReceptacle() == nullptr ? 0 : this->getReceptacle()->id) << " and "
			<< (child->getReceptacle() == nullptr ? 0 : child->getReceptacle()->id) << "\n");
		/* Always merge with the root in the disjoint-set forest */
		if (this != this->getReference()) {
			this->getReference()->merge(child);
		} else {
			child = child->getReference();
			/* If the other blossoms tree in the disjoint-set data
			* structure is bigger, merge into this blossom instead.
			* -> Union-by-size! */
			if (child->complexity > this->complexity) {
				child->setReceptacle(this->receptacle);
				child->merge(this);
			} else {
				child->setReceptacle(nullptr);
				this->childBlossoms.append(&child->childElement);
				child->parentBlossom = this;
				this->complexity += child->complexity;
				Statistics::processMComp(this->complexity);
			}
		}
	}

	/* Removes a child from the disjoint-set data structure */
	template <class Label>
	void CherryBlossom<Label>::remove(CherryBlossom<Label>* child) {
		this->childBlossoms.remove(&child->childElement);
		child->parentBlossom = nullptr;
		this->complexity -= child->complexity;
	}

	/* Returns the representant of this blossom in the disjoint-set data structure */
	template <class Label>
	CherryBlossom<Label>* CherryBlossom<Label>::getReference() {
		if (this->parentBlossom == nullptr) {
			return this;
		} else {
			this->parentBlossom = this->parentBlossom->getReference();
			return this->parentBlossom;
		}
	}

	template <class Label>
	void CherryBlossom<Label>::updateLevel() {
		this->level = this->receptacle->getLevel();
	}
	template <class Label>
	int CherryBlossom<Label>::getLevel() {
		return this->getReference()->level;
	}
	template <class Label>
	int CherryBlossom<Label>::getComplexity() {
		return this->getReference()->complexity;
	}
}