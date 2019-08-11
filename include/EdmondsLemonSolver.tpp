#include "EdmondsLemonSolver.h"

namespace maxmatching {
	template <class Label>
	EdmondsLemonSolver<Label>::EdmondsLemonSolver()
		: vertices()
		, vertexMap()
		, inverseVertexMap() {
		this->graph = new Graph();
		this->matcher = new Matcher(*this->graph);
	}

	template <class Label>
	EdmondsLemonSolver<Label>::~EdmondsLemonSolver() {
		this->clearVertices();
		delete(this->matcher);
		delete(this->graph);
	}

	template <class Label>
	void EdmondsLemonSolver<Label>::addVertex(EdmondsVertex<Label>* v) {
		Graph::Node node = this->graph->addNode();
		this->vertexMap[v] = node;
		this->inverseVertexMap[node] = v;
		this->vertices.push_back(v);
	}

	template <class Label>
	void EdmondsLemonSolver<Label>::addEdge(EdmondsVertex<Label>* u, EdmondsVertex<Label>* v) {
		this->graph->addEdge(this->vertexMap[u], this->vertexMap[v]);
	}

	template <class Label>
	void EdmondsLemonSolver<Label>::calculateMaxMatching() {
		this->matcher->run();
	}

	template <class Label>
	std::vector<EdmondsVertex<Label>*>* EdmondsLemonSolver<Label>::getMatchingRepresentatives() {
		auto ret = new std::vector<EdmondsVertex<Label>*>();
		for (auto ev : this->vertices) {
			auto lv = this->vertexMap[ev];
			auto lm = this->matcher->mate(lv);
			if (lm != lemon::INVALID) {
				auto em = this->inverseVertexMap[lm];
				if (ev->id < em->id) {
					ret->push_back(ev);
				}
			}
		}
		return ret;
	}

	template <class Label>
	std::vector<std::pair<Label, Label>>* EdmondsLemonSolver<Label>::getMatchingLabels() {
		auto ret = new std::vector<std::pair<Label, Label>>();
		for (auto ev : this->vertices) {
			auto lv = this->vertexMap[ev];
			auto lm = this->matcher->mate(lv);
			if (lm != lemon::INVALID) {
				auto em = this->inverseVertexMap[lm];
				if (ev->id < em->id) {
					ret->push_back({ ev->label, em->label });
				}
			}
		}
		return ret;
	}

	template <class Label>
	void EdmondsLemonSolver<Label>::reset() {
		delete(this->matcher);
		this->matcher = new Matcher(*this->graph);
	}

	template <class Label>
	void EdmondsLemonSolver<Label>::clearVertices() {
		this->graph->clear();
		while (!this->vertices.empty()) {
			EdmondsVertex<Label>* v = this->vertices.back();
			this->vertices.pop_back();
			delete(v);
		}
		this->inverseVertexMap.clear();
		this->vertexMap.clear();
	}
}