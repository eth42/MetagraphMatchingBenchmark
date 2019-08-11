#include "EdmondsBoostSolver.h"

namespace maxmatching {
	template <class Label>
	EdmondsBoostSolver<Label>::EdmondsBoostSolver()
		: vertices()
		, invertedVMap()
		, graph()
		, mateMap() {}


	template <class Label>
	EdmondsBoostSolver<Label>::~EdmondsBoostSolver() {}


	template <class Label>
	void EdmondsBoostSolver<Label>::addVertex(EdmondsVertex<Label>* v) {
		auto descriptor = this->graph.add_vertex(v);
		this->invertedVMap[descriptor] = v;
		this->vertices.push_back(v);
	}

	template <class Label>
	void EdmondsBoostSolver<Label>::addEdge(EdmondsVertex<Label>* u, EdmondsVertex<Label>* v) {
		boost::add_edge_by_label(u, v, this->graph);
	}

	template <class Label>
	void EdmondsBoostSolver<Label>::calculateMaxMatching() {
		auto mateMapWrapper = boost::associative_property_map<MateMap>(this->mateMap);
		boost::edmonds_maximum_cardinality_matching(this->graph, mateMapWrapper);
		/* This disables the sorting of vertices and edges before execution. It is necessary
		 * for the gabow worst-case benchmark. Simply replace the above line by this code, if
		 * you want to run this benchmark. */
		 /*boost::matching<
			 Graph,
			 MateMapW,
			 typename boost::property_map<Graph,boost::vertex_index_t>::type,
			 boost::edmonds_augmenting_path_finder,
			 boost::greedy_matching,
			 boost::no_matching_verifier>
			 (this->graph, mateMapWrapper, boost::get(boost::vertex_index, this->graph));*/
	}

	template <class Label>
	std::vector<EdmondsVertex<Label>*>* EdmondsBoostSolver<Label>::getMatchingRepresentatives() {
		std::vector<EdmondsVertex<Label>*>* ret = new std::vector<EdmondsVertex<Label>*>();
		for (auto it = this->mateMap.begin(); it != this->mateMap.end(); it++) {
			auto vh = it->first;
			auto mh = it->second;
			if (mh != boost::graph_traits<Graph>::null_vertex()) {
				auto v = this->invertedVMap[vh];
				auto m = this->invertedVMap[mh];
				if (v->id < m->id) {
					ret->push_back(v);
				}
			}
		}
		return ret;
	}

	template <class Label>
	std::vector<std::pair<Label, Label>>* EdmondsBoostSolver<Label>::getMatchingLabels() {
		auto ret = new std::vector<std::pair<Label, Label>>();
		for (auto it = this->mateMap.begin(); it != this->mateMap.end(); it++) {
			auto vh = it->first;
			auto mh = it->second;
			if (mh != boost::graph_traits<Graph>::null_vertex()) {
				auto v = this->invertedVMap[vh];
				auto m = this->invertedVMap[mh];
				if (v->id < m->id) {
					ret->push_back({ v->label, m->label });
				}
			}
		}
		return ret;
	}

	template <class Label>
	void EdmondsBoostSolver<Label>::reset() {
		mateMap = MateMap();
	}

	template <class Label>
	void EdmondsBoostSolver<Label>::clearVertices() {
		while (!this->vertices.empty()) {
			EdmondsVertex<Label>* v = this->vertices.back();
			this->vertices.pop_back();
			VHandle handle = this->graph.vertex(v);
			boost::clear_vertex(handle, this->graph);
			boost::remove_vertex(v, this->graph);
			this->invertedVMap[handle] = nullptr;
			delete(v);
		}
	}

}