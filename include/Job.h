#pragma once
#include <vector>
#include <sstream>
#include "GraphSource.h"
#include "TypeEnums.h"

namespace maxmatching {
	/* Wrapper class to add additional information to a graph source */
	class Job {
	private:
		GraphSource* source;
	public:
		unsigned long seed;
		bool shuffle;
		unsigned int iterations;
		SolverType solver;
		unsigned int solverArg1;
		unsigned int solverArg2;

		Job();
		virtual ~Job();

		GraphSource& getSource();
		void setSource(GraphSource* source);

		virtual bool isCompound();
		virtual Job* nextSubJob();

		std::string createCsvHeader();
		std::string createCsvData();
	};

	/* Wrapper class to combine multiple jobs */
	class JobCollection : public Job {
	private:
		std::vector<Job*> jobs;
	public:
		JobCollection();
		~JobCollection();

		void addJob(Job* job);

		virtual bool isCompound();
		virtual Job* nextSubJob();
	};
}
