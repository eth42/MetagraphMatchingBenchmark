#include "wr/MCherryBlossom.h"

namespace maxmatching {
namespace wr {

	template <class Label>
	MCherryBlossom<Label>::MCherryBlossom(MVertex<Label>* receptacle)
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
	MCherryBlossom<Label>::~MCherryBlossom() {
		DEBUG("Deleting blossom " << (this->receptacle == nullptr ? 0 : this->receptacle->id) << "@" << this << "\n");
		/* Clear all children blossoms */
		while (!this->childBlossoms.isEmpty()) {
			MCherryBlossom<Label>* child = this->childBlossoms.getFirstElement()->value;
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
			MVertex<Label>* v = this->vertices.popElem()->value;
			v->setContainingBlossom(nullptr);
		}
		Statistics::incrementBlosDeleted();
	}

	/* Sets the receptacle.
	 * Also manages the bearingBlossoms lists of the old and new receptacle. */
	template <class Label>
	void MCherryBlossom<Label>::setReceptacle(MVertex<Label>* newReceptacle) {
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
	MVertex<Label>* MCherryBlossom<Label>::getReceptacle() {
		if (this->parentBlossom == nullptr) {
			return this->receptacle;
		} else {
			return this->getReference()->getReceptacle();
		}
	}

	/* Adds a vertex to the cherry blossoms corolla.
	 * Removes the vertex from any other corolla containing this vertex. */
	template <class Label>
	void MCherryBlossom<Label>::add(MVertex<Label>* v) {
		if (v->getImmediateBlossom() != nullptr) {
			v->getImmediateBlossom()->remove(v);
		}
		this->vertices.append(&v->listElemBlossomVertices);
		v->setContainingBlossom(this);
	}

	/* Removes a vertex from the blossom */
	template <class Label>
	void MCherryBlossom<Label>::remove(MVertex<Label>* v) {
		/* Since we're not necessarily flattening blossom structures,
		 * we need to remove the vertex indirectly */
		if (v->getContainingBlossom() == this->getReference()) {
			v->listElemBlossomVertices.containingList->remove(&v->listElemBlossomVertices);
			v->setContainingBlossom(nullptr);
		}
	}

	/* Rotates the blossom according to the pseudocode, except we're
	 * using HalfEdges instead of vertices in the lists here.
	 * The adaptation is mostly trivial though. */
	template <class Label>
	void MCherryBlossom<Label>::rotate(MVertex<Label>* newReceptacle) {
		if (newReceptacle->getContainingBlossom() != this) {
			return;
		}
		DEBUG("Rotating blossom " << this->receptacle->id << " to " << newReceptacle->id << "\n");
		/* The old receptacle has to be added to the corolla.
		* This has to be done in MCherryTree::rotate! */
		List<HalfEdge<Label>> * evenPath = newReceptacle->createEvenPathToReceptacle(this);
		List<HalfEdge<Label>> * oddPath = newReceptacle->createOddPathToReceptacle(this);
		auto evenEndElem = evenPath->getLastElement()->prvElem;
		auto oddEndElem = oddPath->getLastElement()->prvElem;
		if (evenEndElem->value == oddEndElem->value) {
			while (evenEndElem->value->start == oddEndElem->value->start) {
				evenEndElem = evenEndElem->prvElem;
				oddEndElem = oddEndElem->prvElem;
			}
			MVertex<Label>* intermediateRotate = evenEndElem->value->end;
			DEBUG("Intermediate ");
			evenPath->deleteStructure();
			oddPath->deleteStructure();
			this->rotate(intermediateRotate);
			this->rotate(newReceptacle);
			return;
		}
		this->receptacle->bearingBlossoms.remove(&this->listElement);
		this->remove(newReceptacle);
		while (!evenPath->isEmpty()) {
			HalfEdge<Label>* e1 = evenPath->pop();
			HalfEdge<Label>* e2 = evenPath->pop();
			MVertex<Label>* u = e1->start;
			MVertex<Label>* s = e2->start;
			MVertex<Label>* t = e2->end;
			u->setEvenParent(e1);
			s->setEvenParent(e1->inverse);
			s->setMatchingPartner(e2);
			t->setMatchingPartner(e2->inverse);
		}
		evenPath->deleteStructure();
		HalfEdge<Label>* e = oddPath->pop();
		e->end->setEvenParent(e->inverse);
		while (!oddPath->isEmpty()) {
			//HalfEdge<Label>* e1 = oddPath->pop();
			(void)oddPath->pop();
			HalfEdge<Label>* e2 = oddPath->pop();
			MVertex<Label>* s = e2->start;
			MVertex<Label>* t = e2->end;
			if (s->getMatchingPartner() != t) {
				s->setEvenParent(e2);
				t->setEvenParent(e2->inverse);
			}
		}
		oddPath->deleteStructure();
		newReceptacle->setEvenParent(nullptr);
		newReceptacle->setMatchingPartner(nullptr);
		this->add(this->receptacle);
		this->receptacle = newReceptacle;
		this->level = newReceptacle->getLevel();
		newReceptacle->bearingBlossoms.append(&this->listElement);
	}

	/* Helper method to iterate over all vertices */
	template <class Label>
	void MCherryBlossom<Label>::foreachVertex(const std::function<void(MVertex<Label>*)>& fun) {
		for (auto el = this->vertices.getFirstElement(); el != nullptr; el = el->nxtElem) {
			fun(el->value);
		}
		for (auto el = this->childBlossoms.getFirstElement(); el != nullptr; el = el->nxtElem) {
			el->value->foreachVertex(fun);
		}
	}

	/* Basically the "join"/"union" in the disjoint-set data structure */
	template <class Label>
	void MCherryBlossom<Label>::merge(MCherryBlossom<Label>* child) {
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
	void MCherryBlossom<Label>::remove(MCherryBlossom<Label>* child) {
		this->childBlossoms.remove(&child->childElement);
		child->parentBlossom = nullptr;
		this->complexity -= child->complexity;
	}

	/* Returns the representant of this blossom in the disjoint-set data structure */
	template <class Label>
	MCherryBlossom<Label>* MCherryBlossom<Label>::getReference() {
		if (this->parentBlossom == nullptr) {
			return this;
		} else {
			this->parentBlossom = this->parentBlossom->getReference();
			return this->parentBlossom;
		}
	}

	template <class Label>
	void MCherryBlossom<Label>::updateLevel() {
		this->level = this->getReceptacle()->getLevel();
		DEBUG("Updating level at blossom " << this->getReceptacle()->id << " @ " << this << " to " << this->getLevel() << "\n");
	}
	template <class Label>
	int MCherryBlossom<Label>::getLevel() {
		return this->getReference()->level;
	}
	template <class Label>
	int MCherryBlossom<Label>::getComplexity() {
		return this->getReference()->complexity;
	}
}
}
