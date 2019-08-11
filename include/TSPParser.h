#pragma once
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <functional>
#include <array>
#include <iostream>

#include "Tools.h"

namespace maxmatching {
	/* Parser from the .tsp file format to SimpleGraph. I suggest never touching this again ._. */
	class TSPParser {
	private:
		struct AdjMatrix {
			int nVertices;
			std::vector<double> dists;
			std::function<double(unsigned int, unsigned int)> accessor;
		};

		typedef std::array<double, 2> doublet;
		typedef std::array<double, 3> triplet;

		enum NODE_TYPES {
			D2, D3, NNIL
		};
		enum EDGE_TYPES {
			FULL, ENIL, UR, LR, UDR, LDR, UC, LC, UDC, LDC
		};

		static const std::string FIELD_NAME;
		static const std::string FIELD_DESCRIPTION;
		static const std::string FIELD_NVERTICES;
		static const std::string FIELD_DATA_TYPE;
		static const std::string FIELD_NODE_TYPE;
		static const std::string FIELD_EDGE_TYPE;
		static const std::string NODE_2D;
		static const std::string NODE_3D;
		static const std::string NODE_NONE;
		static const std::string EDGE_EXPLICIT;
		static const std::string EDGE_FULL;
		static const std::string EDGE_FUNCTION;
		static const std::string EDGE_UR;
		static const std::string EDGE_LR;
		static const std::string EDGE_UDR;
		static const std::string EDGE_LDR;
		static const std::string EDGE_UC;
		static const std::string EDGE_LC;
		static const std::string EDGE_UDC;
		static const std::string EDGE_LDC;
		static const std::string EDGE_NONE;
		static const std::string AREA_NODES;
		static const std::string AREA_EDGES;
		static const std::string MARKER_EOF;

		void printNearestNeighbors(const std::vector<double>& dists, const unsigned int& v);

		template<class TupleType>
		void readNodes(std::stringstream& nodes, std::vector<TupleType>& tuples);

		template<class TupleType>
		void parseNodes(std::stringstream& nodeBuffer);

		void readEdges(std::stringstream& edges, EDGE_TYPES type, AdjMatrix& adjMatrix);
		void readAdjMatrix(std::stringstream& edges, unsigned int size, AdjMatrix& adjMatrix);
		void readFullEdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void readUREdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void readLREdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void readUDREdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void readLDREdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void readUCEdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void readLCEdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void readUDCEdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void readLDCEdges(std::stringstream& edges, AdjMatrix& adjMatrix);
		void parseAdjMatrix(const AdjMatrix& adjMatrix);

		std::ifstream* in;
		std::ofstream* out;
		unsigned int nNeighbors;
		unsigned int nVertices;

	public:
		TSPParser();
		~TSPParser();

		void parseFile(const std::string& input, const std::string& output, const unsigned int& nNeighbors);
	};

	template<class TupleType>
	void TSPParser::readNodes(std::stringstream& nodes, std::vector<TupleType>& tuples) {
		unsigned int id;
		unsigned int i = 0;
		while (i < this->nVertices && nodes >> id) {
			for (unsigned int d = 0; d < tuples[i].size(); d++) {
				double coord;
				if (nodes >> coord) {
					tuples[i][d] = coord;
				} else {
					break;
				}
			}
			i++;
		}
	}

	template<class TupleType>
	void TSPParser::parseNodes(std::stringstream& nodeBuffer) {
		std::vector<TupleType> nodes(this->nVertices);
		this->readNodes(nodeBuffer, nodes);
		for (unsigned int x = 0; x < this->nVertices; x++) {
			std::vector<double> dists(this->nVertices);
			for (unsigned int y = 0; y < this->nVertices; y++) {
				dists[y] = Vectors<TupleType>::distance(nodes[x], nodes[y]);
			}
			this->printNearestNeighbors(dists, x);
		}
	}
}