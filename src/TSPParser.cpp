#include "TSPParser.h"

namespace maxmatching {
	const std::string TSPParser::FIELD_NAME("NAME");
	const std::string TSPParser::FIELD_DESCRIPTION("COMMENT");
	const std::string TSPParser::FIELD_NVERTICES("DIMENSION");
	const std::string TSPParser::FIELD_DATA_TYPE("EDGE_WEIGHT_TYPE");
	const std::string TSPParser::FIELD_NODE_TYPE("NODE_COORD_TYPE");
	const std::string TSPParser::FIELD_EDGE_TYPE("EDGE_WEIGHT_FORMAT");
	const std::string TSPParser::NODE_2D("TWOD_COORDS");
	const std::string TSPParser::NODE_3D("THREED_COORDS");
	const std::string TSPParser::NODE_NONE("NO_COORDS");
	const std::string TSPParser::EDGE_EXPLICIT("EXPLICIT");
	const std::string TSPParser::EDGE_FULL("FULL_MATRIX");
	const std::string TSPParser::EDGE_FUNCTION("FUNCTION");
	const std::string TSPParser::EDGE_UR("UPPER_ROW");
	const std::string TSPParser::EDGE_LR("LOWER_ROW");
	const std::string TSPParser::EDGE_UDR("UPPER_DIAG_ROW");
	const std::string TSPParser::EDGE_LDR("LOWER_DIAG_ROW");
	const std::string TSPParser::EDGE_UC("UPPER_COL");
	const std::string TSPParser::EDGE_LC("LOWER_COL");
	const std::string TSPParser::EDGE_UDC("UPPER_DIAG_COL");
	const std::string TSPParser::EDGE_LDC("LOWER_DIAG_COL");
	const std::string TSPParser::AREA_NODES("NODE_COORD_SECTION");
	const std::string TSPParser::AREA_EDGES("EDGE_WEIGHT_SECTION");
	const std::string TSPParser::MARKER_EOF("EOF");



	TSPParser::TSPParser()
		: in(nullptr)
		, out(nullptr)
		, nNeighbors(0)
		, nVertices(0) {}


	TSPParser::~TSPParser() {}

	void TSPParser::parseFile(const std::string& input, const std::string& output, const unsigned int& nNeighbors) {
		this->nNeighbors = nNeighbors;
		this->in = new std::ifstream(input);
		this->out = new std::ofstream(output);
		NODE_TYPES nodes = NODE_TYPES::NNIL;
		EDGE_TYPES edges = EDGE_TYPES::ENIL;
		std::string name, description;
		bool isNodeArea = false;
		bool isEdgeArea = false;
		std::stringstream nodeBuffer, edgeBuffer;
		bool hasNodeDefinitions = false;
		bool hasEdgeDefinitions = false;
		std::regex startsWithNumber("^\\s*[0-9].*");
		for (std::string line; std::getline(*this->in, line);) {
			if (isNodeArea) {
				if (std::regex_match(line, startsWithNumber)) {
					hasNodeDefinitions = true;
					nodeBuffer << line << " ";
				} else {
					isNodeArea = false;
				}
			}
			if (isEdgeArea) {
				if (std::regex_match(line, startsWithNumber)) {
					hasEdgeDefinitions = true;
					edgeBuffer << line << " ";
				} else {
					isEdgeArea = false;
				}
			}
			if (Strings::startsWith(line, TSPParser::FIELD_NAME)) {
				Strings::removePrefix(line, TSPParser::FIELD_NAME, name);
			} else if (Strings::startsWith(line, TSPParser::FIELD_DESCRIPTION)) {
				Strings::removePrefix(line, TSPParser::FIELD_DESCRIPTION, description);
			} else if (Strings::startsWith(line, TSPParser::FIELD_NVERTICES)) {
				std::string buffer;
				Strings::removePrefix(line, TSPParser::FIELD_NVERTICES, buffer);
				nVertices = atoi(buffer.c_str());
			} else if (Strings::startsWith(line, TSPParser::FIELD_DATA_TYPE) && nodes == NODE_TYPES::NNIL) {
				std::string dataType;
				Strings::removePrefix(line, TSPParser::FIELD_DATA_TYPE, dataType);
				if (Strings::startsWith(dataType, TSPParser::EDGE_EXPLICIT)) {
					edges = EDGE_TYPES::FULL;
				} else {
					nodes = NODE_TYPES::D2;
				}
			} else if (Strings::startsWith(line, TSPParser::FIELD_NODE_TYPE)) {
				std::string nodeType;
				Strings::removePrefix(line, TSPParser::FIELD_NODE_TYPE, nodeType);
				if (Strings::startsWith(nodeType, TSPParser::NODE_2D)) {
					nodes = NODE_TYPES::D2;
				} else if (Strings::startsWith(nodeType, TSPParser::NODE_3D)) {
					nodes = NODE_TYPES::D3;
				} else if (Strings::startsWith(nodeType, TSPParser::NODE_NONE)) {
					nodes = NODE_TYPES::NNIL;
				}
			} else if (Strings::startsWith(line, TSPParser::FIELD_EDGE_TYPE) && nodes == NODE_TYPES::NNIL) {
				std::string edgeType;
				Strings::removePrefix(line, TSPParser::FIELD_EDGE_TYPE, edgeType);
				if (Strings::startsWith(edgeType, TSPParser::EDGE_FUNCTION)) {
					edges = EDGE_TYPES::ENIL;
				}
#define PARSER_TEST_EDGET(TYPE) \
/**/		else if(Strings::startsWith(edgeType, TSPParser::EDGE_##TYPE)) { \
/**/			edges = EDGE_TYPES::TYPE; \
/**/		}
				/**/PARSER_TEST_EDGET(FULL)
					PARSER_TEST_EDGET(UR)
					PARSER_TEST_EDGET(LR)
					PARSER_TEST_EDGET(UDR)
					PARSER_TEST_EDGET(LDR)
					PARSER_TEST_EDGET(UC)
					PARSER_TEST_EDGET(LC)
					PARSER_TEST_EDGET(UDC)
					PARSER_TEST_EDGET(LDC)
#undef PARSER_TEST_EDGET
			} else if (Strings::startsWith(line, TSPParser::AREA_NODES)) {
					isNodeArea = true;
				} else if (Strings::startsWith(line, TSPParser::AREA_EDGES)) {
					isEdgeArea = true;
				} else if (Strings::startsWith(line, TSPParser::MARKER_EOF)) {
					break;
				}
		}
		*this->out << this->nVertices << "\n";
		if (hasNodeDefinitions) {
			switch (nodes) {
				case NODE_TYPES::D2:
				case NODE_TYPES::NNIL:
					this->parseNodes<doublet>(nodeBuffer);
					break;
				case NODE_TYPES::D3:
					this->parseNodes<triplet>(nodeBuffer);
					break;
			}
		} else if (hasEdgeDefinitions) {
			AdjMatrix adjMatrix;
			this->readEdges(edgeBuffer, edges, adjMatrix),
				this->parseAdjMatrix(adjMatrix);
		}
		this->out->flush();
		this->in->close();
		this->out->close();
		delete(this->in);
		delete(this->out);
		this->in = nullptr;
		this->out = nullptr;
	}

	void TSPParser::printNearestNeighbors(const std::vector<double>& dists, const unsigned int& v) {
		auto distsCpy = dists;
		std::sort(distsCpy.begin(), distsCpy.end());
		double pivot = distsCpy[this->nNeighbors];
		for (unsigned int w = 0; w < this->nVertices; w++) {
			if (w != v && dists[w] <= pivot) {
				*this->out << v << " " << w << "\n";
			}
		}
	}

	void TSPParser::readEdges(std::stringstream& edges, EDGE_TYPES type, AdjMatrix& adjMatrix) {
		switch (type) {
			case EDGE_TYPES::FULL:
			case EDGE_TYPES::ENIL:
				this->readFullEdges(edges, adjMatrix);
				break;
#define PARSER_MAKE_CASE(SPECIFIER) \
/**/	case EDGE_TYPES::SPECIFIER: \
/**/		this->read##SPECIFIER##Edges(edges, adjMatrix); \
/**/		break;
				PARSER_MAKE_CASE(UR);
				PARSER_MAKE_CASE(LR);
				PARSER_MAKE_CASE(UDR);
				PARSER_MAKE_CASE(LDR);
				PARSER_MAKE_CASE(UC);
				PARSER_MAKE_CASE(LC);
				PARSER_MAKE_CASE(UDC);
				PARSER_MAKE_CASE(LDC);
#undef PARSER_MAKE_CASE
		}
	}

	void TSPParser::readAdjMatrix(std::stringstream& edges, unsigned int size, AdjMatrix& adjMatrix) {
		adjMatrix.nVertices = this->nVertices;
		adjMatrix.dists = std::vector<double>(size);
		unsigned int i = 0;
		double dist;
		while (i < size && edges >> dist) {
			adjMatrix.dists[i] = dist;
			i++;
		}
	}

#define SET_ACCESSOR(mat, indexFormula) \
/**/mat.accessor = [&mat](unsigned int x, unsigned int y) -> double { \
/**/	if (x == y) { \
/**/		return 0; \
/**/	} \
/**/	unsigned int index = indexFormula; \
/**/	if (index < mat.dists.size()){ \
/**/		return mat.dists[index]; \
/**/	}else{ \
/**/		return INFINITY; \
/**/	} \
/**/};

	void TSPParser::readFullEdges(std::stringstream& edges, AdjMatrix& adjMatrix) {
		unsigned int nvs = this->nVertices;
		unsigned int size = nvs * nvs;
		this->readAdjMatrix(edges, size, adjMatrix);
		SET_ACCESSOR(adjMatrix, y * adjMatrix.nVertices + x);
	}
	void TSPParser::readUREdges(std::stringstream & edges, AdjMatrix & adjMatrix) {
		unsigned int nvs = this->nVertices;
		unsigned int size = (nvs * nvs - nvs) / 2;
		this->readAdjMatrix(edges, size, adjMatrix);
		SET_ACCESSOR(adjMatrix,
			(x > y ?
				y * adjMatrix.nVertices - ((y + 3) * y / 2) - 1 + x :
				x * adjMatrix.nVertices - ((x + 3) * x / 2) - 1 + y));
	}
	void TSPParser::readLREdges(std::stringstream & edges, AdjMatrix & adjMatrix) {
		unsigned int nvs = this->nVertices;
		unsigned int size = (nvs * nvs - nvs) / 2;
		this->readAdjMatrix(edges, size, adjMatrix);
		SET_ACCESSOR(adjMatrix,
			(x > y ?
			(x * x - x) / 2 + y :
				(y * y - y) / 2 + x));
	}
	void TSPParser::readUDREdges(std::stringstream & edges, AdjMatrix & adjMatrix) {
		unsigned int nvs = this->nVertices;
		unsigned int size = (nvs * nvs - nvs) / 2 + nvs;
		this->readAdjMatrix(edges, size, adjMatrix);
		SET_ACCESSOR(adjMatrix,
			(x > y ?
				y * adjMatrix.nVertices - (y * y + y) / 2 + x :
				x * adjMatrix.nVertices - (x * x + x) / 2 + y));
	}
	void TSPParser::readLDREdges(std::stringstream & edges, AdjMatrix & adjMatrix) {
		unsigned int nvs = this->nVertices;
		unsigned int size = (nvs * nvs - nvs) / 2 + nvs;
		this->readAdjMatrix(edges, size, adjMatrix);
		SET_ACCESSOR(adjMatrix,
			(x > y ?
			(x * x - x) / 2 + x + y :
				(y * y - y) / 2 + y + x));
	}
#undef SET_ACCESSOR
	/* Let's fake the rest, it doesn't matter if we transpose the matrix or not */
	void TSPParser::readUCEdges(std::stringstream & edges, AdjMatrix & adjMatrix) {
		this->readLREdges(edges, adjMatrix);
	}
	void TSPParser::readLCEdges(std::stringstream & edges, AdjMatrix & adjMatrix) {
		this->readUREdges(edges, adjMatrix);
	}
	void TSPParser::readUDCEdges(std::stringstream & edges, AdjMatrix & adjMatrix) {
		this->readLDREdges(edges, adjMatrix);
	}
	void TSPParser::readLDCEdges(std::stringstream & edges, AdjMatrix & adjMatrix) {
		this->readUDREdges(edges, adjMatrix);
	}

	void TSPParser::parseAdjMatrix(const TSPParser::AdjMatrix & adjMatrix) {
		std::vector<unsigned int> idxs(this->nVertices);
		std::vector<double> dists(this->nVertices);
		unsigned int maxI = 0;
		double maxDist = 0;
		bool rememberMax = false;
		for (unsigned int y = 0; y < this->nVertices; y++) {
			idxs.clear();
			dists.clear();
			rememberMax = false;
			for (unsigned int x = 0; x < this->nVertices; x++) {
				if (x == y) {
					continue;
				}
				double dist = adjMatrix.accessor(x, y);
				if (idxs.size() < this->nVertices) {
					idxs.push_back(x);
					dists.push_back(dist);
				} else {
					if (!rememberMax) {
						maxI = 0;
						maxDist = dists[0];
						for (unsigned int i = 1; i < this->nVertices; i++) {
							if (dists[i] > maxDist) {
								maxI = i;
								maxDist = dists[i];
							}
						}
					}
					if (maxDist > dist) {
						idxs[maxI] = x;
						dists[maxI] = dist;
						rememberMax = false;
					} else {
						rememberMax = true;
					}
				}
			}
			for (unsigned int i = 0; i < idxs.size(); i++) {
				*this->out << y << " " << idxs[i] << "\n";
			}
		}
	}
}