#include "DimacsGenerator.h"

namespace maxmatching {

	DimacsGenerator::~DimacsGenerator() {}

	/* Has to generate a random int in [1..max] */
	unsigned long DimacsGenerator::rand_long(unsigned long max, std::mt19937_64& rgen) {
		return (unsigned long)(rand_d(rgen) * (max - 1)) + 1;
	}
	/* Has to generate a random double in [0..1] */
	double DimacsGenerator::rand_d(std::mt19937_64& rgen) {
		return rgen() / ((double)rgen.max());
	}

	void DimacsGenerator::insert(unsigned long arc) {
		treenode* current = NULL;
		treenode* parent = NULL;
		if (arc_size == 0) {  /* first insertion */
			root = (treenode*)malloc(1 * sizeof(treenode));
			root->val = arc;
			root->left = NULL;
			root->right = NULL;
		} else {
			current = root;
			while (current != NULL) {
				parent = current;
				/* search for the empty spot, saving parent */
				if (arc < parent->val) {
					current = parent->left;
				} else if (arc > parent->val) {
					current = parent->right;
				}
			}/* while */
			current = (treenode*)malloc(1 * sizeof(treenode));
			current->val = arc;
			current->left = NULL;
			current->right = NULL;
			if (arc < (parent->val)) {
				parent->left = current;
			} else {
				parent->right = current;
			}
		}/*else not new  */
		arc_size++;
	}

	bool DimacsGenerator::inset(unsigned long arc) {
		treenode* current;
		current = root;
		while (current != NULL) {
			if (current->val > arc) {
				current = current->left;
			} else if (current->val < arc) {
				current = current->right;
			} else {
				return true;
			}
		}/*while*/
		return false;
	}

	void DimacsGenerator::generateEdgesSparse(SimpleGraph<unsigned int> * graph, unsigned long nEdges, std::mt19937_64 & rgen) {
		unsigned long src, dst, a;
		//unsigned int cost;
		unsigned long nNodes = graph->getVertexCount();
		unsigned long maxArcs = ((nNodes * nNodes) - nNodes);
		for (unsigned long i = 0; i < nEdges; i++) {
			/* loop until an unused edge is found */
			/* and dont allow the upper diagonal */
			do {
				a = rand_long(maxArcs, rgen);
				src = ((a - 1) / (nNodes - 1)) + 1;		/* row number in 1..n */
				dst = (a % (nNodes - 1)) + 1;			/* col number in 1..n-1 */
				if (src == dst) {						/* move arc [i,i] to [i,n] */
					dst = nNodes;
				}
			} while ((src > dst) || inset(a));
			/* found one */
			insert(a);
			//cost = rand_long(MAXCOST, rgen);
			graph->addEdgeSym(src - 1, dst - 1);
		}
	}

	void DimacsGenerator::generateEdgesDense(SimpleGraph<unsigned int> * graph, unsigned long nEdges, std::mt19937_64 & rgen) {
		unsigned int nNodes = graph->getVertexCount();
		unsigned long maxArcs = ((nNodes * nNodes) - nNodes);
		unsigned long maxEdges = maxArcs / 2;
		//unsigned int cost;
		double need = (double)nEdges;
		double total = (double)maxEdges;
		double have = 0.0;    /* increment each time one is printed */
		double seen = 0.0;    /* increment each time one is examined */
		/* consider each edge in turn and decide whether to use it */
		for (unsigned long src = 1; src <= nNodes; src++) {
			for (unsigned long dst = 1; dst < src; dst++) {
				if (have >= need) {
					break;
				}
				double x = rand_d(rgen);
				/* choose current edge with prob =  */
				/* (number left to choose)/(number left in set) */
				if ((total - seen) * x < (need - have)) {
					//cost = rand_long(MAXCOST, rgen);
					graph->addEdge(src - 1, dst - 1);
					have++;
				}
				seen++;
			}
		}
	}

	SimpleGraph<unsigned int>* DimacsGenerator::generate(unsigned long nPoints, unsigned long nEdges, std::mt19937_64 & rgen) {
		SimpleGraph<unsigned int>* ret = new SimpleGraph<unsigned int>();
		for (unsigned long i = 0; i < nPoints; i++) {
			ret->addVertex(i);
		}
		/*  use different generation routines depending on density */
		unsigned long maxArcs = ((nPoints * nPoints) - nPoints);
		if (nEdges < maxArcs / 8) {
			generateEdgesSparse(ret, nEdges, rgen);
		} else {
			generateEdgesDense(ret, nEdges, rgen);
		}
		/* Clean up */
		std::vector<treenode*> treeNodes;
		treeNodes.push_back(root);
		for (unsigned long i = 0; i < treeNodes.size(); i++) {
			treenode* n = treeNodes[i];
			if (n->left != NULL) {
				treeNodes.push_back(n->left);
			}
			if (n->right != NULL) {
				treeNodes.push_back(n->right);
			}
		}
		for (treenode* n : treeNodes) {
			free(n);
		}
		arc_size = 0;
		root = NULL;
		return ret;
	}
}
