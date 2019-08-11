#pragma once
#include "SimpleGraph.h"

/* Adopted from http://archive.dimacs.rutgers.edu/pub/netflow/generators/matching/random.c */

/*-----------------------------------------------------------------*/
/*  Generates random undirected graphs with $m$ edges, in          */
/*  Challenge format.  Instances have random uniform edge          */
/*  costs.                                                         */
/*  C. McGeoch at DIMACS, July 1991  */

/*  You may need to insert your own random number generators.     */
/*     double drand48();  returns doubles from (0.0, 1.0]         */
/*     void srand48(seed);  initializes RNG with seed             */
/*     long seed;                                                 */

/*  Example input:         */
/*  nodes 1000             */
/*  edges  2500            */
/*  maxcost 1000           */
/*  seed  818182717        */

/*  Seed command is optional.                           */
/*  nodes  N  : specifies N nodes                       */
/*  edges  A  : specifies A edges                       */
/*  maxcost X : specifies maximum edge weight           */
/*  seed   S  : RNG seed; default system timer          */
/*------------------------------------------------------*/

#define MAXNODES 10000
#define MAXCOST 100000

namespace maxmatching {
	class DimacsGenerator {
	private:
		/* BST for Set searches */
		typedef struct node_type {
			unsigned long val;
			struct node_type* left;
			struct node_type* right;
		} treenode;

		unsigned long arc_size;
		treenode* root;

		unsigned long rand_long(unsigned long max, std::mt19937_64& rgen);
		double rand_d(std::mt19937_64& rgen);
		void insert(unsigned long arc);
		bool inset(unsigned long arc);

		void generateEdgesSparse(SimpleGraph<unsigned int>* graph, unsigned long nEdges, std::mt19937_64& rgen);
		void generateEdgesDense(SimpleGraph<unsigned int>* graph, unsigned long nEdges, std::mt19937_64& rgen);
	public:
		inline DimacsGenerator() : arc_size(0), root(NULL) {}
		~DimacsGenerator();

		SimpleGraph<unsigned int>* generate(unsigned long nPoints, unsigned long nEdges, std::mt19937_64& rgen);
	};

}
