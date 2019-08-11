#include "BlossomVSolver.h"

namespace maxmatching {
	template <class Label>
	BlossomVSolver<Label>::BlossomVSolver() : BlossomVSolver(0,0) {}


	template <class Label>
	BlossomVSolver<Label>::~BlossomVSolver() {
		this->reset();
	}


	template <class Label>
	void BlossomVSolver<Label>::addVertex(BlossomVertex<Label>* v) {
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
	void BlossomVSolver<Label>::addEdge(BlossomVertex<Label>* u, BlossomVertex<Label>* v) {
		this->edges.push_back({ u, v });
		this->edgeWeights.push_back(1);
		if (this->duplicateInstance) {
			this->edges.push_back({ u->mirror, v->mirror });
			this->edgeWeights.push_back(1);
		}
	}

	template <class Label>
	void BlossomVSolver<Label>::calculateMaxMatching() {
		PerfectMatching pm(this->vertices.size(), this->edges.size());
		auto it = this->edges.begin();
		auto wit = this->edgeWeights.begin();
		for (; it != this->edges.end(); it++, wit++) {
			auto e = *it;
			unsigned int w = *wit;
			BlossomVertex<Label>* u = e.first;
			BlossomVertex<Label>* v = e.second;
			pm.AddEdge(u->id, v->id, -1*w);
		}
		PerfectMatching::Options options;
		options.verbose = false;
		pm.options = options;
		pm.Solve();
		this->reset();
		for (unsigned int i = 0; i < this->vertices.size(); i++) {
			unsigned int j = pm.GetMatch(i);
			if (i < j) {
				auto u = this->vertices[i];
				auto v = this->vertices[j];
				if (!this->duplicateInstance || (u->mirror != v && i % 2 == 0)) {
					this->matches.push_back({ u,v });
				}
			}
		}
	}

	template <class Label>
	std::vector<BlossomVertex<Label>*>* BlossomVSolver<Label>::getMatchingRepresentatives() {
		std::vector<BlossomVertex<Label>*>* ret = new std::vector<BlossomVertex<Label>*>();
		for (unsigned int i = 0; i < this->matches.size(); i++) {
			ret->push_back(this->matches[i].first);
		}
		return ret;
	}

	template <class Label>
	std::vector<std::pair<Label, Label>>* BlossomVSolver<Label>::getMatchingLabels() {
		auto ret = new std::vector<std::pair<Label, Label>>();
		for (unsigned int i = 0; i < this->matches.size(); i++) {
			auto u = this->matches[i].first;
			auto v = this->matches[i].second;
			ret->push_back({ u->label, v->label });
		}
		return ret;
	}

	template <class Label>
	void BlossomVSolver<Label>::reset() {
		this->matches.clear();
	}

	template <class Label>
	void BlossomVSolver<Label>::clearVertices() {
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
