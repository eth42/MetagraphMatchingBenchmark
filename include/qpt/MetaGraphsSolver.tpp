#include "qpt/MetaGraphsSolver.h"
/* Macros to help with adressing a lower triangular matrix */
#define MEDGE_MAT_ID_H(ID1, ID2)\
/**/(ID1*ID1-ID1)/2+ID2
#define MEDGE_MAT_ID(ID1, ID2) (ID1 < ID2 ? MEDGE_MAT_ID_H(ID2,ID1): MEDGE_MAT_ID_H(ID1,ID2))

namespace maxmatching {
namespace qpt {

	template <class Label>
	MetaGraphsSolver<Label>::MetaGraphsSolver(unsigned int maxMetaNeighbors, unsigned int maxMetaNeighborsShrinking)
		: vertices()
		, frustratedTrees()
		, growQueueStack(nullptr)
		, frustratedShrinkableStack(nullptr)
		, metaEdgeMatrix()
		, isCalculated(false)
		, I(0.0)
		, RI(0.0)
		, maxMetaNeighbors(maxMetaNeighbors)
		, currentMinNeighbors(0)
		, maxMetaNeighborsShrinking(maxMetaNeighborsShrinking)
		, currentMinNeighborsShrinking(0)
		, nUnmatchedNodes(0)
		, metaEdges(0)
		, preSortStrat(MinDegree) {
		growQueueStack = new List<MCherryTree<Label>> * [maxMetaNeighbors + 1];
		frustratedShrinkableStack = new List<MCherryTree<Label>> * [maxMetaNeighborsShrinking + 1];
		for (unsigned int i = 0; i <= maxMetaNeighbors; i++) {
			growQueueStack[i] = new List<MCherryTree<Label>>();
		}
		for (unsigned int i = 0; i <= maxMetaNeighborsShrinking; i++) {
			frustratedShrinkableStack[i] = new List<MCherryTree<Label>>();
		}
	}

	/* 1k meta edges should be near convergence and small enough not to hurt too badly */
	template <class Label>
	MetaGraphsSolver<Label>::MetaGraphsSolver() : MetaGraphsSolver(1000) {};

	/* Destructor will not clean vertices and edges.
	 * Use clearVertices() before to avoid memory leaks,
	 * if you do not intend to use the vertices afterwards */
	template <class Label>
	MetaGraphsSolver<Label>::~MetaGraphsSolver() {
		for (unsigned int i = 0; i <= maxMetaNeighbors; i++) {
			delete(growQueueStack[i]);
		}
		for (unsigned int i = 0; i <= maxMetaNeighborsShrinking; i++) {
			delete(frustratedShrinkableStack[i]);
		}
		delete[](growQueueStack);
		delete[](frustratedShrinkableStack);
	}

	/* Clear the vertex list and also delete all contained vertices */
	template <class Label>
	void MetaGraphsSolver<Label>::clearVertices() {
		for (MVertex<Label>* v : this->vertices) {
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
				for (MVertex<Label>* v : vertices) {
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
				for (MVertex<Label>* v : vertices) {
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
		for (MVertex<Label>* v : this->vertices) {
			DEBUG("Trying to match " << v->id << "\n");
			if (v->getMatchingPartner() == nullptr) {
				for (HalfEdge<Label>* e : v->neighbors) {
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
			DEBUG("Adding meta edge between " << u->id << " and " << v->id << "\n");
			HalfEdge<Label> * e = new HalfEdge<Label>(u, v, label);
			HalfEdge<Label> * ei = new HalfEdge<Label>(v, u, (label == nullptr ? nullptr : label->inverse));
			e->inverse = ei;
			ei->inverse = e;
			u->neighbors.push_back(e);
			v->neighbors.push_back(ei);
			this->metaEdgeMatrix[id] = true;
			this->metaEdges++;
			/* In principle this would be a nice thing to do, but since we're gonna add u later on anyways, this is somewhat redundant */
			//this->storeInCorrectList(&u->getRepresentedTree()->listElem);
			this->storeInCorrectList(&v->getRepresentedTree()->listElem);
		}
	}

	template <class Label>
	std::vector<HalfEdge<Label>*> * MetaGraphsSolver<Label>::getMatching() {
		std::vector<HalfEdge<Label>*>* ret = new std::vector<HalfEdge<Label>*>();
		std::vector<MVertex<Label>*>* buffer = this->getMatchingRepresentatives();
		for (MVertex<Label>* v : buffer) {
			ret->push_back(v->getMatchingPartnerEdge());
		}
		delete(buffer);
		return ret;
	}

	template <class Label>
	std::vector<MVertex<Label>*>* MetaGraphsSolver<Label>::getMatchingRepresentatives() {
		std::vector<MVertex<Label>*>* ret = new std::vector<MVertex<Label>*>();
		for (MVertex<Label>* v : this->vertices) {
			if (v->getMatchingPartner() != nullptr && v->id < v->getMatchingPartner()->id) {
				ret->push_back(v);
			}
		}
		return ret;
	}

	template <class Label>
	std::vector<std::pair<Label, Label>>* MetaGraphsSolver<Label>::getMatchingLabels() {
		std::vector<std::pair<Label, Label>>* ret = new std::vector<std::pair<Label, Label>>();
		std::vector<MVertex<Label>*>* reps = this->getMatchingRepresentatives();
		for (MVertex<Label>* v : *reps) {
			ret->push_back({ v->label, v->getMatchingPartner()->label });
		}
		delete(reps);
		return ret;
	}

	template <class Label>
	void MetaGraphsSolver<Label>::reset() {
		for (unsigned int i = 0; i <= this->maxMetaNeighbors; i++) {
			this->growQueueStack[i]->clear();
		}
		for (unsigned int i = 0; i <= this->maxMetaNeighborsShrinking; i++) {
			this->frustratedShrinkableStack[i]->clear();
		}
		this->frustratedTrees.clear();
		for (MVertex<Label>* v : this->vertices) {
			v->reset();
		}
		this->I = 0.0;
		this->RI = 0.0;
		isCalculated = false;
	}

	template <class Label>
	void MetaGraphsSolver<Label>::storeInCorrectList(ListElement<MCherryTree<Label>> * treeEl) {
		if (treeEl->containingList != nullptr) {
			treeEl->containingList->remove(treeEl);
		}
		if (treeEl->value->growQueue.isEmpty()) {
			if (treeEl->value->canCreateBlossom()) {
				const unsigned int nNeighbors = treeEl->value->metaVertex->neighbors.size();
				if (nNeighbors >= this->maxMetaNeighborsShrinking) {
					this->frustratedShrinkableStack[this->maxMetaNeighborsShrinking]->append(treeEl);
				} else {
					this->frustratedShrinkableStack[nNeighbors]->append(treeEl);
				}
			} else {
				this->frustratedTrees.append(treeEl);
			}
		} else {
			const unsigned int nNeighbors = treeEl->value->metaVertex->neighbors.size();
			if (nNeighbors >= this->maxMetaNeighbors) {
				this->growQueueStack[this->maxMetaNeighbors]->append(treeEl);
			} else {
				this->growQueueStack[nNeighbors]->append(treeEl);
			}
		}
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
		this->nUnmatchedNodes = 0;
		MVertex<Label>::resetIds();
		for (MVertex<Label>* v : this->vertices) {
			if (v->getMatchingPartner() == nullptr) {
				MCherryTree<Label>* tree = new MCherryTree<Label>(v);
				//this->remainingTrees.append(&tree->listElem);
				this->growQueueStack[0]->append(&tree->listElem);
				v->enqueue();
				this->nUnmatchedNodes++;
			}
		}
		/* Loop until no progress is made, i.e. no additional matchings are found */
		//bool keepRunning = this->remainingTrees.getSize() > 1;
		while (this->nUnmatchedNodes > 1 && ((this->growQueueStack[0]->getSize() > 0) || (this->frustratedShrinkableStack[0]->getSize() > 0))) {
			//int nRemainingTrees = this->remainingTrees.getSize();
			//this->metaEdgeMatrix.resize(nRemainingTrees * (nRemainingTrees - 1) / 2, false);
			this->metaEdgeMatrix.resize(this->nUnmatchedNodes * (this->nUnmatchedNodes - 1) / 2, false);
			this->I++;
			this->RI++;
			//std::cout << this->I << ": " << this->nUnmatchedNodes << " | "
			//	<< this->growQueueStack[0]->getSize() << " + "
			//	<< this->frustratedShrinkableStack[0]->getSize() << " + "
			//	<< this->frustratedTrees.getSize() << "\n";
			/* Grow all trees as long as possible */
			this->currentMinNeighbors = 0;
			this->currentMinNeighborsShrinking = 0;
			this->metaEdges = 0;
			ListElement<MCherryTree<Label>> * treeEl = this->getGrowableTreeEl();
			while (treeEl != nullptr) {
				MVertex<Label>* v = treeEl->value->growQueue.popElem()->value;
				this->growFrom(v);
				this->storeInCorrectList(treeEl);
				treeEl = this->getGrowableTreeEl();
			}
			/* Create a solver for the metagraph and initialize it */
			MetaGraphsSolver<Label> metaSolver(this->maxMetaNeighbors, this->maxMetaNeighborsShrinking);
			std::function<void(List<MCherryTree<Label>>*)> useMetaVertices = [&metaSolver](List<MCherryTree<Label>> * tList) {
				for (ListElement<MCherryTree<Label>>* el = tList->getFirstElement(); el != nullptr; el = el->nxtElem) {
					MVertex<Label>* mv = el->value->metaVertex;
					if (mv->neighbors.size() > 0) {
						DEBUG("Using MVertex " << mv->id << "\n");
						metaSolver.addVertex(mv);
					} else {
						DEBUG("Not using MVertex " << mv->id << "\n");
					}
				}
			};
			//for (unsigned int i = 0; i < this->maxMetaNeighbors; i++) {
			//	if (this->growQueueStack[i]->getSize() > 0) {
			//		std::cout << "@" << this << " Wow grow queue: " << i << "\n";
			//		std::exit(1);
			//	}
			//}
			//std::cout << "Shrinkables: " << this->frustratedShrinkableStack[0]->getSize() << "\n";
			useMetaVertices(this->growQueueStack[this->maxMetaNeighbors]);
			for (unsigned int i = 1; i <= this->maxMetaNeighborsShrinking; i++) {
				useMetaVertices(this->frustratedShrinkableStack[i]);
			}
			useMetaVertices(&this->frustratedTrees);
			unsigned int nMetaEdges = 0;
			for (unsigned int i = 0; i < this->nUnmatchedNodes; i++) {
				for (unsigned int j = 0; j < i; j++) {
					int id = MEDGE_MAT_ID(i, j);
					if (this->metaEdgeMatrix[id]) {
						nMetaEdges++;
					}
				}
			}
			//std::cout << "Meta solver has " << metaSolver.vertices.size() << " vertices\n";
			/* Empty meta vertices means no extended matching */
			if (metaSolver.vertices.size() > 0) {
				metaSolver.calculateMaxMatching();
				this->I += metaSolver.getI() * .5;
				this->RI += metaSolver.getRI() * metaSolver.getVertices().size() / this->getVertices().size();
				std::vector<MVertex<Label>*>* matching = metaSolver.getMatchingRepresentatives();
				this->applyMetaMatching(matching);
				delete(matching);
				/* Exit if there are 0 possibly growable trees */
				if ((this->growQueueStack[0]->getSize() > 0) || (this->frustratedShrinkableStack[0]->getSize() > 0)) {
					int newIds = 0;
					std::function<void(List<MCherryTree<Label>>*)> resetVertices = [&newIds](List<MCherryTree<Label>> * tList) {
						for (ListElement<MCherryTree<Label>>* el = tList->getFirstElement(); el != nullptr; el = el->nxtElem) {
							MVertex<Label>* mv = el->value->metaVertex;
							mv->reset();
							mv->clearEdges();
							mv->id = newIds++;
						}
					};
					resetVertices(this->growQueueStack[0]);
					resetVertices(this->frustratedShrinkableStack[0]);
					resetVertices(&this->frustratedTrees);
					this->metaEdgeMatrix.clear();
					this->metaEdgeMatrix.shrink_to_fit();
					DEBUG("Finished resetting alive meta vertices\n");
					this->nUnmatchedNodes = newIds;
				}
			} else {
				break;
			}
		}
		/* Update statistics */
		isCalculated = true;
		DEBUG("Exeting meta graph calculation (I=" << this->getI() << ", RI=" << this->getRI() << ")\n\n");
		Statistics::setCurrentI(this->getI());
		Statistics::setCurrentRI(this->getRI());
	}

	template <class Label>
	ListElement<MCherryTree<Label>>* MetaGraphsSolver<Label>::getGrowableTreeEl() {
		while (this->currentMinNeighbors < this->maxMetaNeighbors) {
			if (!this->growQueueStack[this->currentMinNeighbors]->isEmpty()) {
				return this->growQueueStack[this->currentMinNeighbors]->popElem();
			} else {
				this->currentMinNeighbors++;
			}
		}
		while (this->currentMinNeighborsShrinking < this->maxMetaNeighborsShrinking) {
			if (!this->frustratedShrinkableStack[this->currentMinNeighborsShrinking]->isEmpty()) {
				ListElement<MCherryTree<Label>>* treeEl = this->frustratedShrinkableStack[this->currentMinNeighborsShrinking]->popElem();
				while (treeEl->value->growQueue.isEmpty() && treeEl->value->canCreateBlossom()) {
					treeEl->value->makeBlossom();
				}
				if (!treeEl->value->growQueue.isEmpty()) {
					/* Update min neighbors, since we reintroduce a tree to the normal grow stack */
					this->currentMinNeighbors = this->currentMinNeighborsShrinking;
					return treeEl;
				//} else if (treeEl->value->canCreateBlossom()) {
				//	this->frustratedShrinkableStack[0]->append(treeEl);
				} else {
					this->frustratedTrees.append(treeEl);
				}
			} else {
				this->currentMinNeighborsShrinking++;
			}
		}
		return nullptr;
	}

	template <class Label>
	void MetaGraphsSolver<Label>::growFrom(MVertex<Label> * v) {
		/* Iterate over neighbors according to pseudocode */
		MCherryTree<Label>* tv = v->getContainingTree();
		DEBUG("MVertex " << v->id << " has " << v->neighbors.size() << " neighbors.\n");
		for (HalfEdge<Label>* e : v->neighbors) {
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
					/* In the same tree -> check for blossom creation and potentially store for later use */
					tw->rememberBlossom(e);
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
			tv->listElem.containingList->remove(&tv->listElem);
			tw->listElem.containingList->remove(&tw->listElem);
			twinTrees.append(&tv->listElem);
			twinTrees.append(&tw->listElem);
		}
		/* If only 0 or 1 unmatched vertices remain,
		 * there is no point for reintegration anyways */
		if (this->nUnmatchedNodes - twinTrees.getSize() > 1) {
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
			MCherryTree<Label>* tree = twinTrees->popElem()->value;
			tree->printTree();
			List<MVertex<Label>>* lvs = tree->getVertices();
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
		/*
		 * Shift all growable and shrinkable trees down to 0 meta edges.
		 * It suffices to consider the highest grow queue since either
		 * trees reach this queue or they get frustrated half way through
		 */
		std::function<void(List<MCherryTree<Label>> * *)> listSwap = [this](List<MCherryTree<Label>> * *list) {
			List<MCherryTree<Label>>* buffer = list[this->maxMetaNeighbors];
			list[this->maxMetaNeighbors] = list[0];
			list[0] = buffer;
		};
		listSwap(this->growQueueStack);
		for (unsigned int i = 1; i <= this->maxMetaNeighborsShrinking; i++) {
			while (!this->frustratedShrinkableStack[i]->isEmpty()) {
				this->frustratedShrinkableStack[0]->append(this->frustratedShrinkableStack[i]->popElem());
			}
		}
		//listSwap(this->frustratedShrinkableStack);
		/* Reintegrate the vertices into the remaining trees */
		while (!vs.isEmpty()) {
			MVertex<Label>* s = vs.pop();
			if (s->getContainingTree() != nullptr) {
				/* This has already been moved to a new tree */
				continue;
			}
			/* Insert the vertices if any neighbor is even in another tree */
			for (HalfEdge<Label>* st : s->neighbors) {
				MVertex<Label>* t = st->end;
				if (t->isEven()) {
					MCherryTree<Label>* newTree = t->getContainingTree();
					DEBUG("Adapting " << s->id << " into tree " << newTree->root->id << "\n");
					newTree->add(st->inverse);
					/* Growable trees are not frustrated anymore */
					if (newTree->listElem.containingList != this->growQueueStack[0]) {
						newTree->listElem.containingList->remove(&newTree->listElem);
						this->growQueueStack[0]->append(&newTree->listElem);
					}
					newTree->updateLevelBelow(s->getMatchingPartner());
					newTree->printTree();
					break;
				}
			}
		}
		//std::cout << this->I << ": " << this->nUnmatchedNodes << " | "
		//	<< this->growQueueStack[0]->getSize() << " + "
		//	<< this->frustratedShrinkableStack[0]->getSize() << " + "
		//	<< this->frustratedTrees.getSize() << "\n";
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
