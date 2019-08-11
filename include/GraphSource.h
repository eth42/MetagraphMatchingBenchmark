#pragma once
#include <sstream>
#include "Tools.h"
#include "SimpleGraph.h"
#include "TSPParser.h"
#include "GImporter.h"
#include "GExporter.h"
#include "GGenerator.h"

namespace maxmatching {
	/* Container class for different sources of instances for benchmarking. */
	class GraphSource {
	protected:
		unsigned int size;
		GraphSource(const unsigned int& size);
	public:
		virtual ~GraphSource();
		/* Create the next graph or nullptr if none such graph can be constructed */
		virtual SimpleGraph<unsigned int>* getNext() = 0;
		/* Get a string representation of the source */
		virtual std::string printSource() = 0;
		/* Compound means it has sub sources, i.e. a folder containing files */
		virtual bool isCompound() = 0;
		/* Gets the next sub source, if any. Nullptr otherwise. */
		virtual GraphSource* nextSubSource() = 0;
	};

	/* Will not create any graph and is always empty. */
	class VoidGraphSource : public GraphSource {
	public:
		VoidGraphSource();
		virtual SimpleGraph<unsigned int>* getNext();
		std::string printSource();
		inline bool isCompound() {
			return false;
		}
		inline GraphSource* nextSubSource() {
			return nullptr;
		}
	};

	/* Reads a *.tsp file and returns the contained graph. */
	class FileGraphSource : public GraphSource {
	private:
		std::string source;
		unsigned int nNeighbors;
	public:
		FileGraphSource(const std::string& source, const unsigned int& nNeighbors);
		SimpleGraph<unsigned int>* getNext();
		std::string printSource();
		inline bool isCompound() {
			return false;
		}
		inline GraphSource* nextSubSource() {
			return nullptr;
		}
	};

	/* Scans a folder for *.tsp files and creates a file source for each contained file */
	class FolderGraphSource : public GraphSource {
	private:
		std::string source;
		std::vector<std::string> sources;
		unsigned int nNeighbors;
		std::string lastSource;
	public:
		FolderGraphSource(const std::string& source, const unsigned int& nNeighbors);
		SimpleGraph<unsigned int>* getNext();
		std::string printSource();
		inline bool isCompound() {
			return true;
		}
		GraphSource* nextSubSource();
	};

	/* Uses a graph generator to create benchmarking instances. */
	class GeneratorGraphSource : public GraphSource {
	private:
		std::string tmpFile;
		std::string string;
		std::function<SimpleGraph<unsigned int>* (GGenerator)> generator;
	public:
		GeneratorGraphSource(std::string tmpFile, std::string string, std::function<SimpleGraph<unsigned int>* (GGenerator)> generator);
		~GeneratorGraphSource();
		SimpleGraph<unsigned int>* getNext();
		std::string printSource();
		inline bool isCompound() {
			return false;
		}
		inline GraphSource* nextSubSource() {
			return nullptr;
		}
	};
}