#pragma once

namespace maxmatching {
namespace wr {
	template <class Label>
	class MCherryTree;
}
}

#include <fstream>
#include <sstream>
#include <functional>
#include "Debug.h"
#include "Statistics.h"
#include "List.h"
#include "ListElement.h"
#include "wr/MVertex.h"
#include "wr/HalfEdge.h"

namespace maxmatching {
namespace wr {
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
	public:
		/* Root of the tree */
		MVertex<Label>* root;
		/* Number of vertices in the tree */
		int size;
		/* Metavertex representing the tree in the metagraph */
		MVertex<Label>* metaVertex;
		/* ListElement for the remainingTrees list in the MetaGraphsSolver class
		* Necessary for O(1) deletion */
		ListElement<MCherryTree<Label>> listElem;

		MCherryTree(MVertex<Label>* root);
		~MCherryTree();

		void rotate(MVertex<Label>* newRoot);
		void add(HalfEdge<Label>* parentToChild);
		void remove(MVertex<Label>* v);
		void makeBlossom(HalfEdge<Label>* e);
		void updateLevel();
		void updateLevelBelow(MVertex<Label>* v);

		List<MVertex<Label>>* getVertices();

		void logTree();
		void printTree();
	};

}
}
#include "wr/MCherryTree.tpp"
