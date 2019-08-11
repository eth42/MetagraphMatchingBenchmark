#pragma once

namespace maxmatching {
	template <class Label>
	class CherryTree;
}

#include <sstream>
#include <fstream>
#include <functional>
#include "Statistics.h"
#include "Debug.h"
#include "CherryBlossom.h"
#include "Vertex.h"

namespace maxmatching {
	template <class Label>
	class CherryTree {
	private:
		/* Counter to enumerate debug prints */
		static int printCtr;
		/* Internal debug method */
		void recursivePrintNode(Vertex<Label>* v, const std::string& prefix, std::stringstream& stream);
		/* Adopts a blossom with a receptacle, that has just been added to the tree */
		void adoptBlossom(CherryBlossom<Label>* blossom);
		/* Internal debug flag.
		 * Printing a tree, while the structure is being reformed is unsafe. */
		bool consistent;
	public:
		/* Root of the tree */
		Vertex<Label>* root;
		/* Number of vertices in the tree */
		int size;

		CherryTree(Vertex<Label>* root);
		~CherryTree();

		void rotate(Vertex<Label>* newRoot);
		void add(Vertex<Label>* parent, Vertex<Label>* newVertex);
		void remove(Vertex<Label>* v);
		void makeBlossom(Vertex<Label>* u, Vertex<Label>* w);
		void updateLevel();
		void updateLevelBelow(Vertex<Label>* v);

		List<Vertex<Label>>* getVertices();

		void logTree();
		void printTree();
	};
}

#include "CherryTree.tpp"