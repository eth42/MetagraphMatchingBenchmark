#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <functional>
#include <stdarg.h>
#include <limits.h>
#include "Statistics.h"
#include "List.h"
#include "Vertex.h"
#include "MultiTreeSolver.h"
#include "norm/MVertex.h"
#include "norm/MetaGraphsSolver.h"
#include "qpt/MVertex.h"
#include "qpt/MetaGraphsSolver.h"
#include "wr/MVertex.h"
#include "wr/MetaGraphsSolver.h"
#include "EdmondsVertex.h"
#include "EdmondsBoostSolver.h"
#ifdef HAS_LEMON
#include "EdmondsLemonSolver.h"
#endif
#ifdef HAS_BLOSSOM_IV
#include "BlossomIVSolver.h"
#endif
#ifdef HAS_BLOSSOM_V
#include "BlossomVSolver.h"
#endif
#include "BlossomVertex.h"
#include "GImporter.h"
#include "TSPParser.h"
#include "Tools.h"
#include "GraphSource.h"
#include "TypeEnums.h"
#include "IntStepper.h"
#include "Job.h"

using namespace maxmatching;

bool printBaseGraphs = false;
std::ofstream csv("measure.csv");

template<typename VType, typename LType>
void printMatching(Solver<VType, LType> solver) {
	/* Print labels of matched vertices */
	std::vector<std::pair<LType, LType>>* labels = solver->getMatchingLabels();
	/* Switch lower labels to front */
	for (unsigned int i = 0; i < labels->size(); i++) {
		std::pair<LType, LType> pair = (*labels)[i];
		if (pair.first > pair.second) {
			std::pair<LType, LType> tmp({ pair.second, pair.first });
			(*labels)[i].swap(tmp);
		}
	}
	/* Sort matchings ascending */
	std::sort(labels->begin(), labels->end(),
		[](std::pair<LType, LType> a, std::pair<LType, LType> b) {
			return a.first < b.first;
		});
	/* Print matchings */
	std::cout << "Matchings:\n";
	for (auto pair : *labels) {
		std::cout << pair.first << " " << pair.second << "\n";
	}
	delete(labels);
}

/* Executes a job specified by the user input. If it is a compound job,
 * every job is executed one after another. All data is printed to the csv ofstream. */
template<typename SType, typename VType>
void performJob(Job& job, bool isSub) {
	/* The first call will print a header row for the csv data */
	if (!isSub) {
		csv << job.createCsvHeader() << ", "
			<< "#Vertices, "
			<< "#Edges, "
			<< Statistics::createCsvHeader() << ", "
			<< "Matching\n";
	}
	/* Compound jobs get split up. */
	if (job.isCompound()) {
		Job* subJob = job.nextSubJob();
		while (subJob != nullptr) {
			performJob<SType, VType>(*subJob, true);
			delete(subJob);
			subJob = job.nextSubJob();
		}
	} else {
		/* Iterate over all graphs provided by the source */
		auto graph = job.getSource().getNext();
		while (graph != nullptr) {
			Statistics::reset();
			if (printBaseGraphs) {
				graph->print();
			}
			Solver<VType, unsigned int>* solver = nullptr;
			/* These check for wrong results. If a solver produces more than one
			 * maximum matching for isomorphisms of a graph, it is noted and
			 * printed to std::cerr. */
			int matchingSize = -1, secondMatching = -1;
			bool error = false;
			int lSeed = job.seed;
			for (unsigned int i = 0; i < job.iterations; i++) {
				/* If the graph should be shuffled, shuffle the base graph.
				 * By shuffling the base graph and not the implementation specific graphs,
				 * comparability is guaranteed between different algorithms. */
				if (job.shuffle) {
					DEBUG("Shuffling with seed " << lSeed << "\n");
					std::mt19937_64 g(lSeed);
					graph->shuffle(g);
					if (printBaseGraphs) {
						graph->print();
					}
				}
				Statistics::startMeasure();
				VType::resetIds();
				solver = new SType(job.solverArg1,job.solverArg2);
				solver->readGraph(graph);
				DEBUG("Pre calculation\n");
				/* Time is measured after reading the graph, since building
				 * the data structures is not part of the algorithms.
				 * Besides it would add a hefty bias to the boost implementation,
				 * since it is extremely slow in creating graphs. */
				Statistics::startTimer();
				try {
					solver->calculateMaxMatching();
				} catch (const std::exception & ex) {
					std::cerr << "Crashed! Reason: " << ex.what() << "\n"; std::flush(std::cout);
					exit(3);
				} catch (const std::string & ex) {
					std::cerr << "Crashed! Reason: " << ex << "\n"; std::flush(std::cout);
					exit(2);
				} catch (...) {
					std::cerr << "Crashed! Reason: ???\n"; std::flush(std::cout);
					exit(1);
				}
				auto reps = solver->getMatchingRepresentatives();
				/* Stop the timer after a list containing the matching edges representatives
				 * (ends of a matching edge) are obtained, since this is the result of the algorithms. */
				Statistics::pauseTimer();
				int matching = reps->size();
				delete(reps);
				DEBUG("Post calculation\n");
				/* Make sure the algorithm worked correctly */
				if (matchingSize < 0) {
					matchingSize = matching;
				} else if (matchingSize != matching) {
					error = true;
					secondMatching = matching;
				}
				/* Clean up. Using fresh solvers and vertices every iterations
				 * prevents the algorithms from cheating and reusing some information. */
				solver->clearVertices();
				delete(solver);
				/* Stop the current measure, thus creating an entry for the statistics. */
				Statistics::stopMeasure();
				lSeed++;
			}
			/* Error handling */
			if (error) {
				std::cerr << "Found computation error! Computed both max matchings " << matchingSize << " and " << secondMatching << "!\n";
			}
			/* Print the results of the benchmark to the csv ofstream. */
			csv << job.createCsvData() << ", "
				<< graph->getVertexCount() << ", "
				<< graph->getEdgeCount() << ", "
				<< Statistics::createCsvData() << ", "
				<< matchingSize << "\n";
			std::flush(csv);
			delete(graph);
			graph = job.getSource().getNext();
		}
	}
}

template<typename SType, typename VType>
void performJob(Job& job) {
	performJob<SType, VType>(job, false);
}

GraphSource* makeFilesystemGraphSource(const std::string& input, const unsigned int& nNeighbors) {
	if (Files::isDir(input)) {
		return new FolderGraphSource(input, nNeighbors);
	} else if (Files::isFile(input)) {
		return new FileGraphSource(input, nNeighbors);
	} else if (Files::isFile(input + ".tsp")) {
		return new FileGraphSource(input + ".tsp", nNeighbors);
	} else {
		return new VoidGraphSource();
	}
}

#define MAIN_MAKE_RANDOM_1_ARG_SOURCE(SUFFIX) \
/**/GraphSource* makeRandom##SUFFIX##GraphSource(const unsigned int& nVertices, Job& job) { \
/**/	std::stringstream tmpFile, output; \
/**/	tmpFile << "assets/tmp/rnd/" << #SUFFIX << "_" << nVertices <<"_" << job.seed << ".g"; \
/**/	output << "\"Random "<<#SUFFIX<<" (nVertices=" << nVertices << ")\""; \
/**/	auto gen = [&nVertices, &job](GGenerator gen) -> SimpleGraph<unsigned int>* { \
/**/		auto rgen = Random::makeRandom(job.seed); \
/**/		return gen.createRandom##SUFFIX(nVertices, *rgen); \
/**/	}; \
/**/	return new GeneratorGraphSource(tmpFile.str(), output.str(), gen); \
/**/}
#define MAIN_MAKE_RANDOM_2_ARG_SOURCE(SUFFIX) \
/**/GraphSource* makeRandom##SUFFIX##GraphSource(const unsigned int& nVertices, const unsigned int& nNeighbors, Job& job) { \
/**/	std::stringstream tmpFile, output; \
/**/	tmpFile << "assets/tmp/rnd/" << #SUFFIX << "_" << nVertices <<"_" << nNeighbors <<"_" << job.seed << ".g"; \
/**/	output << "\"Random "<<#SUFFIX<<" (nVertices=" << nVertices << ", nNeighbors=" << nNeighbors << ")\""; \
/**/	auto gen = [&nVertices, &nNeighbors, &job](GGenerator gen) -> SimpleGraph<unsigned int>* { \
/**/		auto rgen = Random::makeRandom(job.seed); \
/**/		return gen.createRandom##SUFFIX(nVertices, nNeighbors, *rgen); \
/**/	}; \
/**/	return new GeneratorGraphSource(tmpFile.str(), output.str(), gen); \
/**/}
#define MAIN_MAKE_RANDOM_EUCLID_SOURCE(DIM) \
/**/MAIN_MAKE_RANDOM_2_ARG_SOURCE(Euclid ## DIM ## d)
MAIN_MAKE_RANDOM_2_ARG_SOURCE(Boost);
MAIN_MAKE_RANDOM_2_ARG_SOURCE(Dimacs);
#ifdef HAS_FADE
MAIN_MAKE_RANDOM_1_ARG_SOURCE(Triangle);
#endif
MAIN_MAKE_RANDOM_EUCLID_SOURCE(2);
MAIN_MAKE_RANDOM_EUCLID_SOURCE(3);
MAIN_MAKE_RANDOM_EUCLID_SOURCE(10);
MAIN_MAKE_RANDOM_EUCLID_SOURCE(20);
#undef MAIN_MAKE_RANDOM_1_ARG_SOURCE
#undef MAIN_MAKE_RANDOM_2_ARG_SOURCE
#undef MAIN_MAKE_RANDOM_EUCLID_SOURCE

GraphSource* makeWorstCaseGabowGraphSource(const unsigned int& m) {
	std::stringstream tmpFile, output;
	tmpFile << "assets/tmp/gabow/" << m << ".g";
	output << "\"Worst Case Gabow (m=" << m << ")\"";
	auto gen = [&m](GGenerator gen) -> SimpleGraph<unsigned int> * {
		return gen.createWorstCaseGabow(m);
	};
	return new GeneratorGraphSource(tmpFile.str(), output.str(), gen);
}

#define MAIN_MAKE_TS_SOURCE(SUFFIX) \
/**/GraphSource* makeTriangleSeries##SUFFIX##GraphSource(const unsigned int& nTriangles) { \
/**/	std::stringstream tmpFile, output; \
/**/	tmpFile << "assets/tmp/triangles/"<<#SUFFIX<<"_" << nTriangles << ".g"; \
/**/	output << "\"Triangle series "<<#SUFFIX<<" (nTriangles=" << nTriangles << ")\""; \
/**/	auto gen = [&nTriangles](GGenerator gen) -> SimpleGraph<unsigned int>* { \
/**/		return gen.createTriangles##SUFFIX(nTriangles); \
/**/	}; \
/**/	return new GeneratorGraphSource(tmpFile.str(), output.str(), gen); \
/**/}

MAIN_MAKE_TS_SOURCE(A)
MAIN_MAKE_TS_SOURCE(B)
#undef MAIN_MAKE_TS_SOURCE

#define MAIN_MAKE_HC_SOURCE(SUFFIX) \
/**/GraphSource* makeHoneyCombs##SUFFIX##GraphSource(const unsigned int& width, const unsigned int& rows) { \
/**/	std::stringstream tmpFile, output; \
/**/	tmpFile << "assets/tmp/honey_combs/"<<#SUFFIX<<"_" << width << "x" << rows << ".g"; \
/**/	output << "\"Honey Combs "<<#SUFFIX<<" (width=" << width << ", rows=" << rows << ")\""; \
/**/	auto gen = [&width, &rows](GGenerator gen) -> SimpleGraph<unsigned int>* { \
/**/		return gen.createHoneyCombs##SUFFIX(width, rows); \
/**/	}; \
/**/	return new GeneratorGraphSource(tmpFile.str(), output.str(), gen); \
/**/}

MAIN_MAKE_HC_SOURCE()
MAIN_MAKE_HC_SOURCE(Plus)
MAIN_MAKE_HC_SOURCE(Caps)
MAIN_MAKE_HC_SOURCE(Inner)
#undef MAIN_MAKE_HC_SOURCE

int main(int argc, char** argv) {
	/* Init job */
	JobCollection job;
	job.shuffle = false;
	job.seed = maxmatching::Time::currentTimeMillis();
	job.solver = MultiTrees;

	SourceType src = SourceType::RandomBoost;
	std::string file;
	IntStepper iterationsStepper, param1Stepper, param2Stepper, algParam1Stepper, algParam2Stepper;
	/* Options */
	for (int i = 1; i < argc; i += 1) {
		if (std::strcmp(argv[i], "-o") == 0) {
			printBaseGraphs = true;
		} else if (std::strcmp(argv[i], "-i") == 0) {
			iterationsStepper = Strings::parseIntStepper(argv[i + 1]);
			i++;
		} else if (std::strcmp(argv[i], "-s") == 0) {
			job.shuffle = true;
		} else if (std::strcmp(argv[i], "-S") == 0) {
			job.shuffle = true;
			job.seed = std::atol(argv[i + 1]);
			i++;
		} else if (std::strcmp(argv[i], "-nS") == 0) {
			job.shuffle = false;
			job.seed = std::atol(argv[i + 1]);
			i++;
		} else if (std::strcmp(argv[i], "-f") == 0) {
			file = argv[i + 1];
			i++;
			param1Stepper = Strings::parseIntStepper(argv[i + 1]);
			i++;
			param2Stepper = IntStepper();
			src = Filesystem;
		}
#define MAIN_READ_SRC_1_ARG(FLAG, CASE) \
/**/	else if(std::strcmp(argv[i], FLAG) == 0) { \
/**/		param1Stepper = Strings::parseIntStepper(argv[i + 1]); \
/**/		i++; \
/**/		param2Stepper = IntStepper(); \
/**/		src = CASE; \
/**/	}
#define MAIN_READ_SRC_2_ARG(FLAG, CASE) \
/**/	else if(std::strcmp(argv[i], FLAG) == 0) { \
/**/		param1Stepper = Strings::parseIntStepper(argv[i + 1]); \
/**/		i++; \
/**/		param2Stepper = Strings::parseIntStepper(argv[i + 1]); \
/**/		i++; \
/**/		src = CASE; \
/**/	}
		MAIN_READ_SRC_2_ARG("-Gr", RandomBoost)
			MAIN_READ_SRC_2_ARG("-Grd", RandomDimacs)
#ifdef HAS_FADE
			MAIN_READ_SRC_1_ARG("-Grt", RandomTriangle)
#endif
			MAIN_READ_SRC_2_ARG("-Gre2", RandomEuclid2d)
			MAIN_READ_SRC_2_ARG("-Gre3", RandomEuclid3d)
			MAIN_READ_SRC_2_ARG("-Gre10", RandomEuclid10d)
			MAIN_READ_SRC_2_ARG("-Gre20", RandomEuclid20d)
			MAIN_READ_SRC_1_ARG("-Gg", WorstCaseGabow)
			MAIN_READ_SRC_1_ARG("-Gta", TriangleSeriesA)
			MAIN_READ_SRC_1_ARG("-Gtb", TriangleSeriesB)
			MAIN_READ_SRC_2_ARG("-Ghc", HoneyCombs)
			MAIN_READ_SRC_2_ARG("-Ghcp", HoneyCombsPlus)
			MAIN_READ_SRC_2_ARG("-Ghcc", HoneyCombsCaps)
			MAIN_READ_SRC_2_ARG("-Ghci", HoneyCombsInner)
#undef MAIN_READ_SRC_1_ARG
#undef MAIN_READ_SRC_2_ARG
#define MAIN_READ_SOLVER(FLAG, CASE) \
/**/	else if(std::strcmp(argv[i], FLAG) == 0) { \
/**/		job.solver = CASE; \
/**/		algParam1Stepper = IntStepper(); \
/**/		algParam2Stepper = IntStepper(); \
/**/	}
#define MAIN_READ_SOLVER_1_ARG(FLAG, CASE) \
/**/	else if(std::strcmp(argv[i], FLAG) == 0) { \
/**/		job.solver = CASE; \
/**/		algParam1Stepper = Strings::parseIntStepper(argv[i + 1]); \
/**/		algParam2Stepper = IntStepper(); \
/**/		i++; \
/**/	}
#define MAIN_READ_SOLVER_2_ARG(FLAG, CASE) \
/**/	else if(std::strcmp(argv[i], FLAG) == 0) { \
/**/		job.solver = CASE; \
/**/		algParam1Stepper = Strings::parseIntStepper(argv[i + 1]); \
/**/		algParam2Stepper = Strings::parseIntStepper(argv[i + 2]); \
/**/		i+=2; \
/**/	}
			MAIN_READ_SOLVER("-mt", MultiTrees)
			MAIN_READ_SOLVER("-mg", MetaGraphs)
			MAIN_READ_SOLVER_1_ARG("-mgwr", MetaGraphsWR)
			MAIN_READ_SOLVER_2_ARG("-mgqpt", MetaGraphsQPT)
			MAIN_READ_SOLVER("-eb", EdmondsBoost)
#ifdef HAS_LEMON
			MAIN_READ_SOLVER("-el", EdmondsLemon)
#endif
#ifdef HAS_BLOSSOM_IV
			MAIN_READ_SOLVER_1_ARG("-biv", BlossomIV)
#endif
#ifdef HAS_BLOSSOM_V
			MAIN_READ_SOLVER_1_ARG("-bv", BlossomV)
#endif
#undef MAIN_READ_SOLVER
	}
	for (int iterations : iterationsStepper) {
		JobCollection* iterSubJob = new JobCollection();
		job.addJob(iterSubJob);
		iterSubJob->iterations = iterations;
		for (int param1 : param1Stepper) {
			for (int param2 : param2Stepper) {
				for (int algParam1 : algParam1Stepper) {
					for (int algParam2 : algParam2Stepper) {
						Job* subJob = new Job();
						iterSubJob->addJob(subJob);
						subJob->solverArg1 = algParam1;
						subJob->solverArg2 = algParam2;
						/* In theory creating new ints here is a memory leak,
						 * but it's happing on a way too small scale to be relevant */
						switch (src) {
#define MAIN_SRC_CASE_1_ARG(CASE, ARG) \
/**/					case CASE: \
/**/						subJob->setSource(make##CASE##GraphSource(ARG)); \
/**/						break;
#define MAIN_SRC_CASE_2_ARG(CASE, ARG1, ARG2) \
/**/					case CASE: \
/**/						subJob->setSource(make##CASE##GraphSource(ARG1, ARG2)); \
/**/						break;
#define MAIN_SRC_CASE_3_ARG(CASE, ARG1, ARG2, ARG3) \
/**/					case CASE: \
/**/						subJob->setSource(make##CASE##GraphSource(ARG1, ARG2, ARG3)); \
/**/						break;
#define MAIN_SRC_CASE_1_INT_ARG(CASE) MAIN_SRC_CASE_1_ARG(CASE, *new unsigned int(param1))
#define MAIN_SRC_CASE_2_INT_ARG(CASE) MAIN_SRC_CASE_2_ARG(CASE, *new unsigned int(param1), *new unsigned int(param2))
#define MAIN_SRC_CASE_HC(SUFFIX) MAIN_SRC_CASE_2_INT_ARG(HoneyCombs##SUFFIX)
							MAIN_SRC_CASE_2_ARG(Filesystem, "assets/" + file, *new unsigned int(param1));
							MAIN_SRC_CASE_3_ARG(RandomBoost, *new unsigned int(param1), *new unsigned int(param2), job);
							MAIN_SRC_CASE_3_ARG(RandomDimacs, *new unsigned int(param1), *new unsigned int(param2), job);
#ifdef HAS_FADE
							MAIN_SRC_CASE_2_ARG(RandomTriangle, *new unsigned int(param1), job);
#endif
							MAIN_SRC_CASE_3_ARG(RandomEuclid2d, *new unsigned int(param1), *new unsigned int(param2), job);
							MAIN_SRC_CASE_3_ARG(RandomEuclid3d, *new unsigned int(param1), *new unsigned int(param2), job);
							MAIN_SRC_CASE_3_ARG(RandomEuclid10d, *new unsigned int(param1), *new unsigned int(param2), job);
							MAIN_SRC_CASE_3_ARG(RandomEuclid20d, *new unsigned int(param1), *new unsigned int(param2), job);
							MAIN_SRC_CASE_1_INT_ARG(WorstCaseGabow);
							MAIN_SRC_CASE_1_INT_ARG(TriangleSeriesA);
							MAIN_SRC_CASE_1_INT_ARG(TriangleSeriesB);
							MAIN_SRC_CASE_HC();
							MAIN_SRC_CASE_HC(Plus);
							MAIN_SRC_CASE_HC(Caps);
							MAIN_SRC_CASE_HC(Inner);
#undef MAIN_SRC_CASE_1_ARG
#undef MAIN_SRC_CASE_3_ARG
#undef MAIN_SRC_CASE_1_INT_ARG
#undef MAIN_SRC_CASE_2_INT_ARG
#undef MAIN_SRC_CASE_HC
						}
					}
				}
			}
		}
	}
	switch (job.solver) {
#define MAIN_MAKE_SOLVER_CASE(TYPE, SOLVER_T, VERTEX_T, JOB) \
/**/	case TYPE: \
/**/		performJob<SOLVER_T<unsigned int>, VERTEX_T<unsigned int>>(JOB); \
/**/		break;
		MAIN_MAKE_SOLVER_CASE(MultiTrees, MultiTreeSolver, Vertex, job);
		MAIN_MAKE_SOLVER_CASE(MetaGraphs, norm::MetaGraphsSolver, norm::MVertex, job);
		MAIN_MAKE_SOLVER_CASE(MetaGraphsWR, wr::MetaGraphsSolver, wr::MVertex, job);
		MAIN_MAKE_SOLVER_CASE(MetaGraphsQPT, qpt::MetaGraphsSolver, qpt::MVertex, job);
		MAIN_MAKE_SOLVER_CASE(EdmondsBoost, EdmondsBoostSolver, EdmondsVertex, job);
#ifdef HAS_LEMON
		MAIN_MAKE_SOLVER_CASE(EdmondsLemon, EdmondsLemonSolver, EdmondsVertex, job);
#endif
#ifdef HAS_BLOSSOM_IV
		MAIN_MAKE_SOLVER_CASE(BlossomIV, BlossomIVSolver, BlossomVertex, job);
#endif
#ifdef HAS_BLOSSOM_V
		MAIN_MAKE_SOLVER_CASE(BlossomV, BlossomVSolver, BlossomVertex, job);
#endif
#undef MAIN_MAKE_SOLVER_CASE
	}
	csv.close();
	std::exit(0);
}
