#pragma once
#include <sstream>
#include <limits>
#include "Tools.h"

#define M_DECLARE(TYPE, FIELD)\
	static std::vector<TYPE> all ## FIELD;\
	static TYPE tot ## FIELD;\
	static TYPE cur ## FIELD;
#define M_DECLARE_CD(TYPE, FIELD)\
	M_DECLARE(TYPE, FIELD ## Created);\
	M_DECLARE(TYPE, FIELD ## Deleted);
#define M_DECLARE_INC(FIELD)\
	static void increment ## FIELD()
#define M_DECLARE_INC_CD(FIELD)\
	M_DECLARE_INC(FIELD ## Created);\
	M_DECLARE_INC(FIELD ## Deleted);
#define M_DECLARE_GETTER(TYPE, FIELD)\
	static TYPE getMin ## FIELD();\
	static TYPE getMax ## FIELD();\
	static TYPE get05Percentile ## FIELD();\
	static TYPE get95Percentile ## FIELD();\
	static TYPE getMedian ## FIELD();\
	static double getAverage ## FIELD();\
	static TYPE getCurrent ## FIELD();
#define M_DECLARE_GETTER_CD(TYPE, FIELD)\
	M_DECLARE_GETTER(TYPE, FIELD ## Created);\
	M_DECLARE_GETTER(TYPE, FIELD ## Deleted);
#define M_DECLARE_SETTER(TYPE, FIELD)\
	static void setCurrent ## FIELD(TYPE v);
#define M_DECLARE_SETTER_CD(TYPE, FIELD)\
	M_DECLARE_SETTER(TYPE, FIELD ## Created);\
	M_DECLARE_SETTER(TYPE, FIELD ## Deleted);

namespace maxmatching {
	/* Container for a set of static functions to gather data during benchmarks */
	class Statistics {
	private:
		Statistics();
		~Statistics();

		static unsigned long nMeasurements;
		static bool processing;
		static bool paused;
		static bool timerRunning;
		static unsigned long startStamp;

		M_DECLARE_CD(unsigned long, Vert);
		M_DECLARE_CD(unsigned long, Edge);
		M_DECLARE_CD(unsigned long, Tree);
		M_DECLARE_CD(unsigned long, Blos);
		M_DECLARE(unsigned long, MComp);
		M_DECLARE(unsigned long, Time);

		M_DECLARE(double, I);
		M_DECLARE(double, RI);

		static void processCurrent();
	public:
		static void reset();
		static void resetCurrent();
		static void startMeasure();
		static void pauseMeasure();
		static void stopMeasure();
		static void startTimer();
		static void pauseTimer();
		static std::string createReport();

		M_DECLARE_INC_CD(Vert);
		M_DECLARE_INC_CD(Edge);
		M_DECLARE_INC_CD(Tree);
		M_DECLARE_INC_CD(Blos);

		static void processMComp(unsigned long comp);

		M_DECLARE_GETTER_CD(unsigned long, Vert);
		M_DECLARE_GETTER_CD(unsigned long, Edge);
		M_DECLARE_GETTER_CD(unsigned long, Tree);
		M_DECLARE_GETTER_CD(unsigned long, Blos);
		M_DECLARE_GETTER(unsigned long, MComp);
		M_DECLARE_GETTER(unsigned long, Time);
		M_DECLARE_GETTER(double, I);
		M_DECLARE_GETTER(double, RI);

		M_DECLARE_SETTER(double, I);
		M_DECLARE_SETTER(double, RI);

		static void sort();
		static std::string createCsvHeader();
		static std::string createCsvData();
	};
}

#undef M_DECLARE
#undef M_DECLARE_CD
#undef M_DECLARE_INC
#undef M_DECLARE_INC_CD
#undef M_DECLARE_GETTER
#undef M_DECLARE_GETTER_CD
#undef M_DECLARE_SETTER
#undef M_DECLARE_SETTER_CD