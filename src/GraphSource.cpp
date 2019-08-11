#include "GraphSource.h"

namespace maxmatching {

	/* The base class does not need a useful implementation */
	GraphSource::GraphSource(const unsigned int& size) : size(size) {}
	GraphSource::~GraphSource() {}

	/* The void source is basically just a fancy nullptr */
	VoidGraphSource::VoidGraphSource() : GraphSource(0) {}
	SimpleGraph<unsigned int>* VoidGraphSource::getNext() {
		return nullptr;
	}
	std::string VoidGraphSource::printSource() {
		return "void";
	}

	FileGraphSource::FileGraphSource(const std::string& source, const unsigned int& nNeighbors)
		: GraphSource(1)
		, source(source)
		, nNeighbors(nNeighbors) {}
	/* Reads a tsp file and buffers the result in a g file.
	 * If a g file with the name already exists, uses this one instead. */
	SimpleGraph<unsigned int>* FileGraphSource::getNext() {
		if (this->size > 0) {
			std::string bufferFile = Files::inputFileToBufferFile(source, this->nNeighbors);
			if (!Files::isFile(bufferFile)) {
				Files::makePathToFile(bufferFile);
				TSPParser parser;
				parser.parseFile(source, bufferFile, this->nNeighbors);
			}
			GImporter importer;
			auto ret = importer.importFile(bufferFile);
			this->size--;
			return ret;
		}
		return nullptr;
	}
	std::string FileGraphSource::printSource() {
		std::stringstream ret;
		ret << "\"File source (file="
			<< Files::getFilename(this->source)
			<< ", nNeighbors="
			<< this->nNeighbors
			<< ")\"";
		return ret.str();
	}


	FolderGraphSource::FolderGraphSource(const std::string& source, const unsigned int& nNeighbors)
		: GraphSource(0)
		, source(source)
		, sources()
		, nNeighbors(nNeighbors)
		, lastSource() {
		Files::getContainedFiles(source, ".*\\.tsp", this->sources);
		this->size = this->sources.size();
		std::reverse(this->sources.begin(), this->sources.end());
	}
	/* Reads the next tsp file in the folder and returns the graph. */
	SimpleGraph<unsigned int>* FolderGraphSource::getNext() {
		if (this->size > 0) {
			this->lastSource = this->sources.back();
			auto ret = FileGraphSource(this->lastSource, this->nNeighbors).getNext();
			this->sources.pop_back();
			this->size--;
			return ret;
		}
		return nullptr;
	}
	std::string FolderGraphSource::printSource() {
		std::stringstream ret;
		ret << "\"Folder source (folder="
			<< this->source
			<< ", nNeighbors="
			<< this->nNeighbors
			<< ")\"";
		return ret.str();
	}
	/* Returns the next tsp file wrapped in a file source. */
	GraphSource* FolderGraphSource::nextSubSource() {
		if (this->size > 0) {
			this->lastSource = this->sources.back();
			auto ret = new FileGraphSource(this->lastSource, this->nNeighbors);
			this->sources.pop_back();
			this->size--;
			return ret;
		}
		return nullptr;
	}


	GeneratorGraphSource::GeneratorGraphSource(std::string tmpFile, std::string string, std::function<SimpleGraph<unsigned int>* (GGenerator)> generator)
		: GraphSource(1)
		, tmpFile(tmpFile)
		, string(string)
		, generator(generator) {}
	GeneratorGraphSource::~GeneratorGraphSource() {}
	/* Creates a graph using the provided generator. Buffers the result in a g file.
	 * If a g file with the name already exists, uses this one instead. */
	SimpleGraph<unsigned int>* GeneratorGraphSource::getNext() {
		if (this->size > 0) {
			SimpleGraph<unsigned int>* ret;
			if (!this->tmpFile.empty() && Files::isFile(this->tmpFile)) {
				GImporter importer;
				ret = importer.importFile(this->tmpFile);
			} else {
				GGenerator gen;
				ret = generator(gen);
				if (!this->tmpFile.empty()) {
					GExporter exporter;
					exporter.writeFile(tmpFile, ret);
				}
			}
			this->size--;
			return ret;
		}
		return nullptr;
	}
	std::string GeneratorGraphSource::printSource() {
		return this->string;
	}
}