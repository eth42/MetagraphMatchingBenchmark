#pragma once

namespace maxmatching {
namespace qpt {
	template <class Label>
	class MCherryTree;
}
}

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <functional>
#include "Debug.h"
#include "Statistics.h"
#include "List.h"
#include "ListElement.h"
#include "qpt/MVertex.h"
#include "qpt/HalfEdge.h"

#define M_CHERRY_TREE_HEAP_TYPE FIBONACCI
//#define M_CHERRY_TREE_HEAP_TYPE PAIRING

#if M_CHERRY_TREE_HEAP_TYPE==FIBONACCI
#include <boost/heap/fibonacci_heap.hpp>
#define M_CHERRY_TREE_HEAP boost::heap::fibonacci_heap
#elif M_CHERRY_TREE_HEAP_TYPE==PAIRING
#include <boost/heap/pairing_heap.hpp>
#define M_CHERRY_TREE_HEAP boost::heap::pairing_heap
#endif

//#define M_CHERRY_TREE_EDGE_COMP MEMORY
#define M_CHERRY_TREE_EDGE_COMP DIFF_DEPTH
//#define M_CHERRY_TREE_EDGE_COMP DEPTH_DIFF

#define M_CHERRY_TREE_EDGE_COMP_DIFF HIGHEST
//#define M_CHERRY_TREE_EDGE_COMP_DIFF LOWEST
#define M_CHERRY_TREE_EDGE_COMP_DEPTH HIGHEST
//#define M_CHERRY_TREE_EDGE_COMP_DEPTH LOWEST

#if M_CHERRY_TREE_EDGE_COMP_DIFF==HIGHEST
#define M_CHERRY_TREE_DIFF_COMP (e1.diff < e2.diff)
#elif M_CHERRY_TREE_EDGE_COMP_DIFF==LOWEST
#define M_CHERRY_TREE_DIFF_COMP (e1.diff > e2.diff)
#endif

#if M_CHERRY_TREE_EDGE_COMP_DEPTH==HIGHEST
#define M_CHERRY_TREE_DEPTH_COMP (e1.depth < e2.depth)
#elif M_CHERRY_TREE_EDGE_COMP_DEPTH==LOWEST
#define M_CHERRY_TREE_DEPTH_COMP (e1.depth > e2.depth)
#endif

namespace maxmatching {
namespace qpt {

	template <class Label>
	class EdgeWrap {
	public:
		HalfEdge<Label>* e;
		unsigned int diff;
		unsigned int depth;

		EdgeWrap(HalfEdge<Label>* edge)
			: e(edge)
			, diff(abs(edge->start->getLevel() - edge->end->getLevel()))
#if M_CHERRY_TREE_EDGE_COMP_DEPTH==HIGHEST
			, depth(std::max(edge->start->getLevel(), edge->end->getLevel()))
#elif M_CHERRY_TREE_EDGE_COMP_DEPTH==LOWEST
			, depth(std::min(edge->start->getLevel(), edge->end->getLevel()))
#endif
		{};
	};

	template <class Label>
	struct edgeCompare {
		/* This implements a "less"-operation, thus
		 * if true e2 is prioritized,
		 * if false e1 is prioritized */
		bool operator() (const EdgeWrap<Label> &e1, const EdgeWrap<Label> &e2) const {
#if M_CHERRY_TREE_EDGE_COMP==DIFF_DEPTH
			return M_CHERRY_TREE_DIFF_COMP || M_CHERRY_TREE_DEPTH_COMP;
#elif M_CHERRY_TREE_EDGE_COMP==DEPTH_DIFF
			return M_CHERRY_TREE_DEPTH_COMP || M_CHERRY_TREE_DIFF_COMP;
#elif M_CHERRY_TREE_EDGE_COMP==MEMORY
			return e1 < e2;
#endif
		}
	};

	template <class Label>
	class MCherryTree {
	private:
		/* Counter to enumerate debug prints */
		static int printCtr;
		/* Internal debug method */
		void recursivePrintNode(MVertex<Label>* v, const std::string& prefix, std::stringstream& stream);
		/* Adopts a blossom with a receptacle, that has just been added to the tree */
		void adoptBlossom(MCherryBlossom<Label>* blossom);
		/* Internal debug flag.
		 * Printing a tree, while the structure is being reformed is unsafe. */
		bool consistent;
		/* List of halfedges which can potentially induce a new blossom for delayed execution */
		M_CHERRY_TREE_HEAP<EdgeWrap<Label>, boost::heap::compare<edgeCompare<Label>>> blossomCandidates;

	public:
		/* Root of the tree */
		MVertex<Label>* root;
		/* Number of vertices in the tree */
		int size;
		/* Metavertex representing the tree in the metagraph */
		MVertex<Label>* metaVertex;
		/* GrowQueue containing all vertices within this tree, available for growth */
		List<MVertex<Label>> growQueue;
		/* ListElement for the remainingTrees list in the MetaGraphsSolver class
		* Necessary for O(1) deletion */
		ListElement<MCherryTree<Label>> listElem;

		explicit MCherryTree(MVertex<Label>* root);
		~MCherryTree();

		void rotate(MVertex<Label>* newRoot);
		void add(HalfEdge<Label>* parentToChild);
		void remove(MVertex<Label>* v);
		bool inducesNewBlossom(HalfEdge<Label>* e);
		void rememberBlossom(HalfEdge<Label>* e);
		void makeBlossom();
		bool canCreateBlossom();
		void updateLevel();
		void updateLevelBelow(MVertex<Label>* v);

		List<MVertex<Label>>* getVertices();

		void logTree();
		void printTree();
	};
}
}

#include "qpt/MCherryTree.tpp"

#undef M_CHERRY_TREE_HEAP_TYPE
#undef M_CHERRY_TREE_HEAP
#undef M_CHERRY_TREE_EDGE_COMP
#undef M_CHERRY_TREE_EDGE_COMP_DIFF
#undef M_CHERRY_TREE_EDGE_COMP_DEPTH
#undef M_CHERRY_TREE_DIFF_COMP
#undef M_CHERRY_TREE_DEPTH_COMP
