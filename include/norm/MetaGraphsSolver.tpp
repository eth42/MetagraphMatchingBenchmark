#include "norm/MetaGraphsSolver.h"

/* Macros to help with adressing an upper triangular matrix */
#define MEDGE_MAT_ID_H(ID1, ID2)\
/**/ID1*this->remainingTrees.getSize()+ID2-(ID1+1)*(ID1+2)/2
#define MEDGE_MAT_ID(ID1, ID2) (ID1 > ID2 ? MEDGE_MAT_ID_H(ID2,ID1): MEDGE_MAT_ID_H(ID1,ID2))

namespace maxmatching {
namespace norm {
	template <class Label>
	MetaGraphsSolver<Label>::MetaGraphsSolver()
		: vertices()
		, remainingTrees()
		, growQueue()
		, metaEdgeMatrix()
		, isCalculated(false)
		, I(0.0)
		, RI(0.0)
		, preSortStrat(MinDegree) {}

	/* Destructor will not clean vertices and edges.
	 * Use clearVertices() before to avoid memory leaks,
	 * if you do not intend to use the vertices afterwards */
	template <class Label>
	MetaGraphsSolver<Label>::~MetaGraphsSolver() {}

	/* Clear the vertex list and also delete all contained vertices */
	template <class Label>
	void MetaGraphsSolver<Label>::clearVertices() {
		for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
			MVertex<Label>* v = *it;
			delete(v);
		}
		this->vertices.clear();
		this->vertices.shrink_to_fit();
	}

	/* Sorts the vertices according to the defined presort strategy */
	template <class Label>
	void MetaGraphsSolver<Label>::preSort() {
		int minId = this->vertices.front()->id;
		switch (this->preSortStrat) {
			case None:
				break;
			case MinDegree:
				std::sort(
					this->vertices.begin(),
					this->vertices.end(),
					[](MVertex<Label> * v1, MVertex<Label> * v2)->bool {
						return v1->neighbors.size() < v2->neighbors.size();
					});
				for (auto it = vertices.begin(); it != vertices.end(); it++) {
					MVertex<Label>* v = *it;
					std::sort(
						v->neighbors.begin(),
						v->neighbors.end(),
						[](HalfEdge<Label> * n1, HalfEdge<Label> * n2)->bool {
							return n1->end->neighbors.size() < n2->end->neighbors.size();
						});
				}
				break;
			case MaxDegree:
				std::sort(
					this->vertices.begin(),
					this->vertices.end(),
					[](MVertex<Label> * v1, MVertex<Label> * v2)->bool {
						return v1->neighbors.size() > v2->neighbors.size();
					});
				for (auto it = vertices.begin(); it != vertices.end(); it++) {
					MVertex<Label>* v = *it;
					std::sort(
						v->neighbors.begin(),
						v->neighbors.end(),
						[](HalfEdge<Label> * n1, HalfEdge<Label> * n2)->bool {
							return n1->end->neighbors.size() > n2->end->neighbors.size();
						});
				}
				break;
		}
		for (unsigned int i = 0; i < this->vertices.size(); i++) {
			MVertex<Label>* v = this->vertices[i];
			v->id = minId + i;
		}
	}

	/* Greedily creates an initial maximum matching */
	template <class Label>
	void MetaGraphsSolver<Label>::greedyPreSolve() {
		for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
			MVertex<Label>* v = *it;
			if (v->getMatchingPartner() == nullptr) {
				for (auto nit = v->neighbors.begin(); nit != v->neighbors.end(); nit++) {
					HalfEdge<Label>* e = *nit;
					MVertex<Label>* w = e->end;
					if (w->getMatchingPartner() == nullptr) {
						v->setMatchingPartner(e);
						w->setMatchingPartner(e->inverse);
						break;
					}
#ifdef DEBUG_F
					if (v->getRepresentedTree() != nullptr) {
						DEBUG(v->id << "(" << v->getRepresentedTree()->root->id << "): "
							<< w->id << "("
							<< w->getRepresentedTree()->root->id << ")"
							<< " already has matching partner "
							<< w->getMatchingPartner()->id << "("
							<< w->getMatchingPartner()->getRepresentedTree()->root->id << ")\n");
					}
#endif
				}
			}
		}
	}

	template <class Label>
	std::vector<MVertex<Label>*>& MetaGraphsSolver<Label>::getVertices() {
		return this->vertices;
	}

	template <class Label>
	void MetaGraphsSolver<Label>::addVertex(MVertex<Label> * v) {
		vertices.push_back(v);
		v->setGrowQueue(&this->growQueue);
	}

	template <class Label>
	void MetaGraphsSolver<Label>::addEdge(MVertex<Label> * u, MVertex<Label> * v) {
		addEdge(u, v, nullptr);
	}

	template <class Label>
	void MetaGraphsSolver<Label>::addMetaEdge(MVertex<Label> * u, MVertex<Label> * v) {
		addMetaEdge(u, v, nullptr);
	}

	template <class Label>
	void MetaGraphsSolver<Label>::addEdge(MVertex<Label> * u, MVertex<Label> * v, HalfEdge<Label> * label) {
		HalfEdge<Label>* e = new HalfEdge<Label>(u, v, label);
		HalfEdge<Label>* ei = new HalfEdge<Label>(v, u, (label == nullptr ? nullptr : label->inverse));
		e->inverse = ei;
		ei->inverse = e;
		u->neighbors.push_back(e);
		v->neighbors.push_back(ei);
	}

	template <class Label>
	void MetaGraphsSolver<Label>::addMetaEdge(MVertex<Label> * u, MVertex<Label> * v, HalfEdge<Label> * label) {
		int id = MEDGE_MAT_ID(u->id, v->id);
		if (!this->metaEdgeMatrix[id]) {
			HalfEdge<Label>* e = new HalfEdge<Label>(u, v, label);
			HalfEdge<Label>* ei = new HalfEdge<Label>(v, u, (label == nullptr ? nullptr : label->inverse));
			e->inverse = ei;
			ei->inverse = e;
			u->neighbors.push_back(e);
			v->neighbors.push_back(ei);
			this->metaEdgeMatrix[id] = true;
		}
	}

	template <class Label>
	std::vector<HalfEdge<Label>*>* MetaGraphsSolver<Label>::getMatching() {
		auto ret = new std::vector<HalfEdge<Label>*>();
		auto buffer = this->getMatchingRepresentatives();
		for (auto it = buffer->begin(); it != buffer->end(); it++) {
			MVertex<Label>* v = *it;
			ret->push_back(v->getMatchingPartnerEdge());
		}
		delete(buffer);
		return ret;
	}

	template <class Label>
	std::vector<MVertex<Label>*>* MetaGraphsSolver<Label>::getMatchingRepresentatives() {
		auto ret = new std::vector<MVertex<Label>*>();
		for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
			MVertex<Label>* v = *it;
			if (v->getMatchingPartner() != nullptr && v->id < v->getMatchingPartner()->id) {
				ret->push_back(v);
			}
		}
		return ret;
	}

	template <class Label>
	std::vector<std::pair<Label, Label>>* MetaGraphsSolver<Label>::getMatchingLabels() {
		auto ret = new std::vector<std::pair<Label, Label>>();
		auto reps = this->getMatchingRepresentatives();
		for (auto v : *reps) {
			ret->push_back({ v->label, v->getMatchingPartner()->label });
		}
		delete(reps);
		return ret;
	}

	template <class Label>
	void MetaGraphsSolver<Label>::reset() {
		while (!remainingTrees.isEmpty()) {
			remainingTrees.popElem();
		}
		for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
			MVertex<Label>* v = *it;
			v->reset();
		}
		this->I = 0.0;
		this->RI = 0.0;
		isCalculated = false;
	}

	template <class Label>
	void MetaGraphsSolver<Label>::calculateMaxMatching() {
		if (isCalculated) {
			reset();
		}
		DEBUG("\nEntering new meta graph calculation\n");
		preSort();
		/* Start with a fast approximation */
		greedyPreSolve();
		DEBUG("Finished greedy approximation\n");
		/* Initialize problem */
		MVertex<Label>::resetIds();
		for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
			MVertex<Label>* v = *it;
			if (v->getMatchingPartner() == nullptr) {
				MCherryTree<Label>* tree = new MCherryTree<Label>(v);
				this->remainingTrees.append(&tree->listElem);
				v->enqueue();
			}
		}
		/* Loop until no progress is made, i.e. no additional matchings are found */
		bool keepRunning = this->remainingTrees.getSize() > 1;
		while (keepRunning) {
			unsigned long nRemainingTrees = this->remainingTrees.getSize();
			this->metaEdgeMatrix.resize(nRemainingTrees * (nRemainingTrees + 1) / 2, false);
			this->I++;
			this->RI++;
			/* Grow all trees as long as possible */
			while (!this->growQueue.isEmpty()) {
				MVertex<Label>* v = this->growQueue.popElem()->value;
				this->growFrom(v);
			}
			/* Create a solver for the metagraph and initialize it */
			MetaGraphsSolver<Label> metaSolver;
			for (auto el = this->remainingTrees.getFirstElement(); el != nullptr; el = el->nxtElem) {
				MVertex<Label>* mv = el->value->metaVertex;
				/* Only use metavertices with neighbors */
				if (mv->neighbors.size() > 0) {
					DEBUG("Using MVertex " << mv->id << "\n");
					metaSolver.addVertex(mv);
				}
#ifdef DEBUG_F
				else {
					DEBUG("Ignoring MVertex " << mv->id << "\n");
				}
#endif
			}
			/* Empty meta vertices means no extended matching */
			if (metaSolver.vertices.size() == 0) {
				keepRunning = false;
			} else {
				metaSolver.calculateMaxMatching();
				this->I += metaSolver.getI() * .5;
				this->RI += metaSolver.getRI() * metaSolver.getVertices().size() / this->getVertices().size();
				auto matching = metaSolver.getMatchingRepresentatives();
				this->applyMetaMatching(matching);
				delete(matching);
				/* Exit if there is 0 or 1 unmatched vertices */
				if (this->remainingTrees.getSize() <= 1) {
					keepRunning = false;
				} else {
					int newIds = 0;
					for (auto el = this->remainingTrees.getFirstElement(); el != nullptr; el = el->nxtElem) {
						MVertex<Label>* mv = el->value->metaVertex;
						mv->reset();
						mv->clearEdges();
						mv->id = newIds++;
					}
					this->metaEdgeMatrix.clear();
					this->metaEdgeMatrix.shrink_to_fit();
					DEBUG("Finished resetting alive meta vertices\n");
				}
			}
		}
		/* Update statistics */
		isCalculated = true;
		DEBUG("Exeting meta graph calculation (I=" << this->getI() << ", RI=" << this->getRI() << ")\n\n");
		Statistics::setCurrentI(this->getI());
		Statistics::setCurrentRI(this->getRI());
	}

	template <class Label>
	void MetaGraphsSolver<Label>::growFrom(MVertex<Label> * v) {
		/* Iterate over neighbors according to pseudocode */
		MCherryTree<Label>* tv = v->getContainingTree();
		DEBUG("MVertex " << v->id << " has " << v->neighbors.size() << " neighbors.\n");
		for (auto it = v->neighbors.begin(); it != v->neighbors.end(); it++) {
			HalfEdge<Label>* e = *it;
			DEBUG(v->id << ", "
				<< (e->start == nullptr ? 0 : e->start->id) << ", "
				<< (e->end == nullptr ? 0 : e->end->id) << "\n");
			MVertex<Label> * w = e->end;
			DEBUG("Checking neighbor " << w->id << "\n");
			if (w == v->getMatchingPartner()) {
				continue;
			}
			MCherryTree<Label>* tw = w->getContainingTree();
			MVertex<Label>* mw = w->getMatchingPartner();
			/* If w is an even node in a cherry tree */
			if (w->isEven()) {
				if (tw == tv) {
					/* In the same tree -> check for blossom creation */
					tw->makeBlossom(e);
				} else {
					/* In another tree -> create metaedge */
					this->addMetaEdge(tv->metaVertex, tw->metaVertex, e);
				}
			} else if (tw == nullptr) {
				/* Add the vertex (and adopt everything still
				 * "hanging on it" from old trees). */
				tv->add(e);
				tv->updateLevelBelow(mw);
			}
		}
		DEBUG("-----\n");
	}

	template <class Label>
	void MetaGraphsSolver<Label>::applyMetaMatching(std::vector<MVertex<Label>*> * matching) {
		DEBUG("Applying meta matching of size " << matching->size() << "\n");
		List<MCherryTree<Label>> twinTrees;
		for (unsigned int i = 0; i < matching->size(); i++) {
			HalfEdge<Label>* vw = (*matching)[i]->getMatchingPartnerEdge()->label;
			MVertex<Label>* v = vw->start;
			MVertex<Label>* w = vw->end;
			MCherryTree<Label>* tv = v->getContainingTree();
			MCherryTree<Label>* tw = w->getContainingTree();
			tv->rotate(v);
			tw->rotate(w);
			v->setMatchingPartner(vw);
			w->setMatchingPartner(vw->inverse);
			this->remainingTrees.remove(&tv->listElem);
			this->remainingTrees.remove(&tw->listElem);
			twinTrees.append(tv);
			twinTrees.append(tw);
		}
		/* If only 0 or 1 unmatched vertices remain,
		 * there is no point for reintegration anyways */
		if (this->remainingTrees.getSize() > 1) {
			this->batchDissolveTrees(&twinTrees);
		}
		DEBUG("Applied meta matching\n");
	}

	template <class Label>
	void MetaGraphsSolver<Label>::batchDissolveTrees(List<MCherryTree<Label>> * twinTrees) {
		/* Collect all vertices of all trees to delete */
		DEBUG("Batch dissolving " << twinTrees->getSize() << " trees.\n");
		List<MVertex<Label>> vs;
		while (!twinTrees->isEmpty()) {
			MCherryTree<Label>* tree = twinTrees->pop();
			tree->printTree();
			auto lvs = tree->getVertices();
			/* Remove references to the old tree */
			while (!lvs->isEmpty()) {
				MVertex<Label>* s = lvs->pop();
				s->dequeue();
				tree->remove(s);
				vs.push(s);
			}
			lvs->deleteStructure();
			delete(tree);
		}
		/* Reintegrate the vertices into the remaining trees */
		while (!vs.isEmpty()) {
			MVertex<Label>* s = vs.pop();
			if (s->getContainingTree() != nullptr) {
				/* This has already been moved to a new tree */
				continue;
			}
			/* Insert the vertices if any neighbor is even in another tree */
			for (auto it = s->neighbors.begin(); it != s->neighbors.end(); it++) {
				HalfEdge<Label>* st = *it;
				MVertex<Label>* t = st->end;
				MCherryTree<Label>* newTree = t->getContainingTree();
				if (t->isEven()) {
					DEBUG("Adapting " << s->id << " into tree " << newTree->root->id << "\n");
					newTree->add(st->inverse);
					newTree->updateLevelBelow(s->getMatchingPartner());
					newTree->printTree();
					break;
				}
			}
		}
		DEBUG("Finished batch dissolving\n");
	}

	template <class Label>
	double MetaGraphsSolver<Label>::getI() {
		return this->I;
	}

	template <class Label>
	double MetaGraphsSolver<Label>::getRI() {
		return this->RI;
	}
}
}
#undef MEDGE_MAT_ID
#undef MEDGE_MAT_ID_H
