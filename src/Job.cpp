#include "Job.h"

namespace maxmatching {
	Job::Job()
		: source(new VoidGraphSource)
		, seed(0)
		, shuffle(false)
		, iterations(0)
		, solver(MultiTrees)
		, solverArg1(0)
		, solverArg2(0) {}


	Job::~Job() {
		delete(this->source);
	}

	GraphSource& Job::getSource() {
		return *this->source;
	}

	void Job::setSource(GraphSource* source) {
		delete(this->source);
		if (source == nullptr) {
			this->source = new VoidGraphSource();
		} else {
			this->source = source;
		}
	}

	bool Job::isCompound() {
		return this->source->isCompound();
	}
	Job* Job::nextSubJob() {
		auto subSource = this->source->nextSubSource();
		if (subSource == nullptr) {
			return nullptr;
		}
		auto ret = new Job(*this);
		ret->source = subSource;
		return ret;
	}


	std::string Job::createCsvHeader() {
		std::stringstream ret;
		ret << "Source, Seed, Shuffle, Iterations, Solver, Solver arg 1, Solver arg 2";
		return ret.str();
	}

	std::string Job::createCsvData() {
		std::stringstream ret;
		ret << source->printSource()
			<< ", " << this->seed
			<< ", " << this->shuffle
			<< ", " << this->iterations
			<< ", ";
		switch (solver) {
#define JOB_PRINT_CASE(TYPE) \
	case TYPE: ret << #TYPE; break;
			JOB_PRINT_CASE(MultiTrees);
			JOB_PRINT_CASE(MetaGraphs);
			JOB_PRINT_CASE(MetaGraphsWR);
			JOB_PRINT_CASE(MetaGraphsQPT);
			JOB_PRINT_CASE(EdmondsBoost);
#ifdef HAS_LEMON
			JOB_PRINT_CASE(EdmondsLemon);
#endif
#ifdef HAS_BLOSSOM_IV
			JOB_PRINT_CASE(BlossomIV);
#endif
#ifdef HAS_BLOSSOM_V
			JOB_PRINT_CASE(BlossomV);
#endif
#undef JOB_PRINT_CASE
		}
		ret << ", " << solverArg1;
		ret << ", " << solverArg2;
		return ret.str();
	}

	JobCollection::JobCollection()
		: Job()
		, jobs() {}

	JobCollection::~JobCollection() {
		while (!this->jobs.empty()) {
			auto job = this->jobs.back();
			this->jobs.pop_back();
			delete(job);
		}
	}
	void JobCollection::addJob(Job* job) {
		this->jobs.push_back(job);
		job->seed = this->seed;
		job->shuffle = this->shuffle;
		job->solver = this->solver;
		job->iterations = this->iterations;
	}
	bool JobCollection::isCompound() {
		return true;
	}
	Job* JobCollection::nextSubJob() {
		if (!this->jobs.empty()) {
			auto job = this->jobs.back();
			this->jobs.pop_back();
			return job;
		}
		return nullptr;
	}
}
