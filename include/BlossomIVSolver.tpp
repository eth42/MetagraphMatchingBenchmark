#include "BlossomIVSolver.h"

extern "C" {
	int perfect_match_mod(int ncount, int ecount, int** elist,
		int** elen, int just_fractional, int no_fractional, int use_all_trees,
		int partialprice, int* matches, int* n_matches);
}

namespace maxmatching {
	template <class Label>
	BlossomIVSolver<Label>::BlossomIVSolver() : BlossomIVSolver(0,0) {
	}

	template <class Label>
	BlossomIVSolver<Label>::~BlossomIVSolver() {
		this->reset();
	}


	template <class Label>
	void BlossomIVSolver<Label>::addVertex(BlossomVertex<Label>* v) {
		this->vertices.push_back(v);
		if (this->duplicateInstance) {
			BlossomVertex<Label>* vm = new BlossomVertex<Label>(v->label);
			vm->mirror = v;
			v->mirror = vm;
			this->vertices.push_back(vm);
			this->edges.push_back({ v, vm });
			this->edgeWeights.push_back(0);
		}
	}

	template <class Label>
	void BlossomIVSolver<Label>::addEdge(BlossomVertex<Label>* u, BlossomVertex<Label>* v) {
		this->edges.push_back({ u, v });
		this->edgeWeights.push_back(1);
		if (this->duplicateInstance) {
			this->edges.push_back({ u->mirror, v->mirror });
			this->edgeWeights.push_back(1);
		}
	}

	template <class Label>
	void BlossomIVSolver<Label>::calculateMaxMatching() {
		this->reset();
		int* elist = new int[2 * this->edges.size()];
		int* elen = new int[this->edges.size()];
		unsigned int i = 0;
		auto it = this->edges.begin();
		auto wit = this->edgeWeights.begin();
		for (; it != this->edges.end(); it++, wit++) {
			auto e = *it;
			unsigned int w = *wit;
			elist[2 * i] = e.first->id;
			elist[2 * i + 1] = e.second->id;
			elen[i] = -1*w;
			i++;
		}
		int just_frac = 0;
		int	no_frac = 1;
		int use_all_trees = 0;
		int partialprice = 0;

		this->matches = new int[this->vertices.size()];
		this->n_matches = 0;

		perfect_match_mod(
			this->vertices.size(),
			this->edges.size(),
			&elist,
			&elen,
			just_frac,
			no_frac,
			use_all_trees,
			partialprice,
			this->matches,
			&this->n_matches);

		delete[](elist);
		delete[](elen);
	}

	template <class Label>
	std::vector<BlossomVertex<Label>*>* BlossomIVSolver<Label>::getMatchingRepresentatives() {
		std::vector<BlossomVertex<Label>*>* ret = new std::vector<BlossomVertex<Label>*>();
		for (int i = 0; i < this->n_matches; i++) {
			auto u = this->vertices[this->matches[2 * i]];
			auto v = this->vertices[this->matches[2 * i + 1]];
			if (!this->duplicateInstance || (u->mirror != v && this->matches[2 * i] % 2 == 0)) {
				ret->push_back(u);
			}
		}
		std::cout << n_matches << "\n";
		std::cout << ret->size() << "\n"; 
		return ret;
	}

	template <class Label>
	std::vector<std::pair<Label, Label>>* BlossomIVSolver<Label>::getMatchingLabels() {
		auto ret = new std::vector<std::pair<Label, Label>>();
		for (int i = 0; i < this->n_matches; i++) {
			auto u = this->vertices[this->matches[2 * i]];
			auto v = this->vertices[this->matches[2 * i + 1]];
			if (!this->duplicateInstance || (u->mirror != v && this->matches[2 * i] % 2 == 0)) {
				ret->push_back({ u->label, v->label });
			}
		}
		return ret;
	}

	template <class Label>
	void BlossomIVSolver<Label>::reset() {
		if (this->matches != nullptr) {
			delete[](this->matches);
			this->matches = nullptr;
			this->n_matches = 0;
		}
	}

	template <class Label>
	void BlossomIVSolver<Label>::clearVertices() {
		this->reset();
		this->edges.clear();
		this->edgeWeights.clear();
		while (!this->vertices.empty()) {
			BlossomVertex<Label>* v = this->vertices.back();
			this->vertices.pop_back();
			delete(v);
		}
	}
}
