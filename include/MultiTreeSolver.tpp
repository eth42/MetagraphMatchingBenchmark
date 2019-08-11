#include "MultiTreeSolver.h"

namespace maxmatching {
	template <class Label>
	MultiTreeSolver<Label>::MultiTreeSolver()
		: vertices()
		, growQueue()
		, isCalculated(false)
		, preSortStrat(MinDegree) {}

	/* Destructor will not clean vertices and edges.
	* Use clearVertices() before to avoid memory leaks,
	* if you do not intend to use the vertices afterwards */
	template <class Label>
	MultiTreeSolver<Label>::~MultiTreeSolver() {}

	/* Clear the vertex list and also delete all contained vertices */
	template <class Label>
	void MultiTreeSolver<Label>::clearVertices() {
		while (!this->vertices.empty()) {
			Vertex<Label>* v = this->vertices.back();
			this->vertices.pop_back();
			delete(v);
		}
		this->vertices.shrink_to_fit();
	}

	/* Sorts the vertices according to the defined presort strategy */
	template <class Label>
	void MultiTreeSolver<Label>::preSort() {
		int minId = this->vertices.front()->id;
		switch (this->preSortStrat) {
			case None:
				break;
			case MinDegree:
				std::sort(
					this->vertices.begin(),
					this->vertices.end(),
					[](Vertex<Label> * v1, Vertex<Label> * v2)->bool {
						return v1->neighbors.size() < v2->neighbors.size();
					});
				for (auto it = vertices.begin(); it != vertices.end(); it++) {
					Vertex<Label>* v = *it;
					std::sort(
						v->neighbors.begin(),
						v->neighbors.end(),
						[](Vertex<Label> * n1, Vertex<Label> * n2)->bool {
							return n1->neighbors.size() < n2->neighbors.size();
						});
				}
				break;
			case MaxDegree:
				std::sort(
					this->vertices.begin(),
					this->vertices.end(),
					[](Vertex<Label> * v1, Vertex<Label> * v2)->bool {
						return v1->neighbors.size() > v2->neighbors.size();
					});
				for (auto it = vertices.begin(); it != vertices.end(); it++) {
					Vertex<Label>* v = *it;
					std::sort(
						v->neighbors.begin(),
						v->neighbors.end(),
						[](Vertex<Label> * n1, Vertex<Label> * n2)->bool {
							return n1->neighbors.size() > n2->neighbors.size();
						});
				}
				break;
		}
		for (unsigned int i = 0; i < this->vertices.size(); i++) {
			Vertex<Label>* v = this->vertices[i];
			v->id = minId + i;
		}
	}

	/* Greedily creates an initial maximum matching */
	template <class Label>
	void MultiTreeSolver<Label>::greedyPreSolve() {
		for (auto it = vertices.begin(); it != vertices.end(); it++) {
			Vertex<Label>* v = *it;
			if (v->getMatchingPartner() == nullptr) {
				for (auto nit = v->neighbors.begin(); nit != v->neighbors.end(); nit++) {
					Vertex<Label>* w = *nit;
					if (w->getMatchingPartner() == nullptr) {
						v->setMatchingPartner(w);
						w->setMatchingPartner(v);
						break;
					}
				}
			}
		}
	}

	template <class Label>
	std::vector<Vertex<Label>*>& MultiTreeSolver<Label>::getVertices() {
		return this->vertices;
	}

	template <class Label>
	void MultiTreeSolver<Label>::addVertex(Vertex<Label>* v) {
		vertices.push_back(v);
		v->setGrowQueue(&this->growQueue);
	}

	template <class Label>
	void MultiTreeSolver<Label>::addEdge(Vertex<Label>* u, Vertex<Label>* v) {
		std::vector<Vertex<Label>*> uns = u->neighbors;
		u->neighbors.push_back(v);
		v->neighbors.push_back(u);
	}

	template <class Label>
	std::vector<std::pair<Vertex<Label>*, Vertex<Label>*>>* MultiTreeSolver<Label>::getMatching() {
		auto ret = new std::vector<std::pair<Vertex<Label>*, Vertex<Label>*>>();
		auto buffer = this->getMatchingRepresentatives();
		for (auto it = buffer->begin(); it != buffer->end(); it++) {
			Vertex<Label>* v = *it;
			ret->push_back(std::pair<Vertex<Label>*, Vertex<Label>*>(v, v->getMatchingPartner()));
		}
		delete(buffer);
		return ret;
	}

	template <class Label>
	std::vector<Vertex<Label>*>* MultiTreeSolver<Label>::getMatchingRepresentatives() {
		auto ret = new std::vector<Vertex<Label>*>();
		for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
			Vertex<Label>* v = *it;
			if (v->getMatchingPartner() != nullptr && v->id < v->getMatchingPartner()->id) {
				ret->push_back(v);
			}
		}
		return ret;
	}

	template <class Label>
	std::vector<std::pair<Label, Label>>* MultiTreeSolver<Label>::getMatchingLabels() {
		auto ret = new std::vector<std::pair<Label, Label>>();
		auto reps = this->getMatchingRepresentatives();
		for (auto v : *reps) {
			ret->push_back({ v->label, v->getMatchingPartner()->label });
		}
		delete(reps);
		return ret;
	}

	template <class Label>
	void MultiTreeSolver<Label>::reset() {
		for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
			Vertex<Label>* v = *it;
			v->reset();
		}
		isCalculated = false;
	}

	template <class Label>
	void MultiTreeSolver<Label>::calculateMaxMatching() {
		if (isCalculated) {
			reset();
		}
		/* Start with a fast approximation */
		preSort();
		greedyPreSolve();
		DEBUG("Finished greedy approximation\n");
		/* Initialize problem */
		for (auto it = this->vertices.begin(); it != this->vertices.end(); it++) {
			Vertex<Label>* v = *it;
			if (v->getMatchingPartner() == nullptr) {
				//CherryTree<Label>* tree = new CherryTree<Label>(v);
				(void) new CherryTree<Label>(v);
				v->enqueue();
			}
		}
		/* Growing */
		while (!this->growQueue.isEmpty()) {
			Vertex<Label>* v = this->growQueue.popElem()->value;
			this->growFrom(v);
		}
		isCalculated = true;
	}

	template <class Label>
	void MultiTreeSolver<Label>::growFrom(Vertex<Label>* v) {
		/* Iterate over neighbors according to pseudocode */
		CherryTree<Label>* tv = v->getContainingTree();
		DEBUG("Vertex " << v->id << " has " << v->neighbors.size() << " neighbors.\n");
		for (auto it = v->neighbors.begin(); it != v->neighbors.end(); it++) {
			Vertex<Label>* w = *it;
			DEBUG("Checking neighbor " << w->id << "\n");
			if (w == v->getMatchingPartner()) {
				continue;
			}
			CherryTree<Label>* tw = w->getContainingTree();
			//Vertex<Label> * mw = w->getMatchingPartner();
			/* If w is an even node in a cherry tree */
			if (w->isEven()) {
				if (tw == tv) {
					/* In the same tree -> check for blossom creation */
					tw->makeBlossom(v, w);
				} else {
					/* In another tree -> rotate trees to a twin tree */
					DEBUG("Rotating and matching " << v->id << " and " << w->id << "\n");
					tw->printTree();
					tv->printTree();
					tw->rotate(w);
					tv->rotate(v);
					v->setMatchingPartner(w);
					w->setMatchingPartner(v);
					this->dissolveTwinTree(tw, tv);
					break;
				}
			} else if (tw == nullptr) {
				/* Add the vertex (and adopt everything still
				 * "hanging on it" from old trees). */
				tv->add(v, w);
				tv->updateLevelBelow(w->getMatchingPartner());
			}
		}
	}

	template <class Label>
	void MultiTreeSolver<Label>::dissolveTwinTree(CherryTree<Label> * lTree, CherryTree<Label> * rTree) {
		/* Collect all vertices of all trees to delete */
		auto vs = new List<Vertex<Label>>();
		for (int i = 0; i < 2; i++) {
			CherryTree<Label>* tree = i == 0 ? lTree : rTree;
			auto lvs = tree->getVertices();
			/* Remove references to the old tree */
			while (!lvs->isEmpty()) {
				Vertex<Label>* s = lvs->pop();
				s->dequeue();
				tree->remove(s);
				vs->push(s);
			}
			delete(lvs);
			delete(tree);
		}
		/* Reintegrate the vertices into the remaining trees */
		while (!vs->isEmpty()) {
			Vertex<Label>* s = vs->pop();
			if (s->getContainingTree() != nullptr) {
				/* This has already been moved to a new tree */
				continue;
			}
			/* Insert the vertices if any neighbor is even in another tree */
			for (auto it = s->neighbors.begin(); it != s->neighbors.end(); it++) {
				Vertex<Label>* t = *it;
				CherryTree<Label>* newTree = t->getContainingTree();
				if (t->isEven() && newTree != lTree && newTree != rTree) {
					DEBUG("Adapting " << s->id << " into tree " << newTree->root->id << "\n");
					newTree->add(t, s);
					newTree->updateLevelBelow(s);
					break;
				}
			}
		}
		delete(vs);
	}
}