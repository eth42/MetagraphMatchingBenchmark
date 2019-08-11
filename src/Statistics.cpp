#include "Statistics.h"

#define M_ULONG_MAX std::numeric_limits<long>::max()
#define M_INFINITY std::numeric_limits<double>::infinity()

#define M_PROCESS_FIELD(FIELD)\
	all ## FIELD.push_back(Statistics::cur ## FIELD);\
	Statistics::tot ## FIELD += Statistics::cur ## FIELD;
#define M_PROCESS_FIELD_CD(FIELD)\
	M_PROCESS_FIELD(FIELD ## Created);\
	M_PROCESS_FIELD(FIELD ## Deleted);

#define M_INIT(TYPE, FIELD)\
	std::vector<TYPE> Statistics::all ## FIELD = std::vector<TYPE>();\
	TYPE Statistics::tot ## FIELD = 0;\
	TYPE Statistics::cur ## FIELD = 0;
#define M_INIT_CD(TYPE, FIELD)\
	M_INIT(TYPE, FIELD ## Created);\
	M_INIT(TYPE, FIELD ## Deleted);

#define M_RESET(FIELD)\
	Statistics::all ## FIELD.clear();\
	Statistics::all ## FIELD.shrink_to_fit();\
	Statistics::tot ## FIELD = 0;
#define M_RESET_CD(FIELD)\
	M_RESET(FIELD ## Created);\
	M_RESET(FIELD ## Deleted);
#define M_RESET_CUR(FIELD)\
	Statistics::cur ## FIELD = 0;
#define M_RESET_CUR_CD(FIELD)\
	M_RESET_CUR(FIELD ## Created);\
	M_RESET_CUR(FIELD ## Deleted)

#define M_INCREMENTER(FIELD)\
	void Statistics::increment ## FIELD (){Statistics::cur ## FIELD ++;}
#define M_INCREMENTER_CD(FIELD)\
	M_INCREMENTER(FIELD ## Created)\
	M_INCREMENTER(FIELD ## Deleted)
#define M_GETTER(TYPE, FIELD)\
	TYPE Statistics::getMin ## FIELD (){return Statistics::all ## FIELD.front();}\
	TYPE Statistics::getMax ## FIELD (){return Statistics::all ## FIELD.back();}\
	TYPE Statistics::get05Percentile ## FIELD (){return Statistics::all ## FIELD[(Statistics::all ## FIELD.size()-1)*0.05];}\
	TYPE Statistics::get95Percentile ## FIELD (){return Statistics::all ## FIELD[(Statistics::all ## FIELD.size()-1)*0.95];}\
	TYPE Statistics::getMedian ## FIELD (){return Statistics::all ## FIELD[(Statistics::all ## FIELD.size()-1)*0.5];}\
	double Statistics::getAverage ## FIELD (){if(Statistics::nMeasurements == 0) return 0; return Statistics::tot ## FIELD / double(Statistics::nMeasurements);}\
	TYPE Statistics::getCurrent ## FIELD (){return Statistics::cur ## FIELD;}
#define M_GETTER_CD(TYPE, FIELD)\
	M_GETTER(TYPE, FIELD ## Created)\
	M_GETTER(TYPE, FIELD ## Deleted)
#define M_SETTER(TYPE, FIELD)\
	void Statistics::setCurrent ## FIELD (TYPE v){Statistics::cur ## FIELD = v;}
#define M_SETTER_CD(TYPE, FIELD)\
	M_SETTER(TYPE, FIELD ## Created)\
	M_SETTER(TYPE, FIELD ## Deleted)
#define M_SORT_ALL(FIELD)\
	std::sort(Statistics::all ## FIELD.begin(), Statistics::all ## FIELD.end());
#define M_SORT_ALL_CD(FIELD)\
	M_SORT_ALL(FIELD ## Created)\
	M_SORT_ALL(FIELD ## Deleted)\

#define M_PRINT(STREAM, FIELD, UNIT)\
	STREAM << #FIELD << " min:\t" << Statistics::getMin ## FIELD() << " " << UNIT << "\n";\
	STREAM << #FIELD << " max:\t" << Statistics::getMax ## FIELD() << " " << UNIT << "\n";\
	STREAM << #FIELD << " avg:\t" << Statistics::getAverage ## FIELD() << " " << UNIT << "\n";
#define M_PRINT_CD(STREAM, FIELD, UNIT)\
	M_PRINT(STREAM, FIELD ## Created, UNIT);\
	M_PRINT(STREAM, FIELD ## Deleted, UNIT);


namespace maxmatching {
	unsigned long Statistics::nMeasurements = 0;
	bool Statistics::processing = false;
	bool Statistics::paused = false;
	bool Statistics::timerRunning = false;
	unsigned long Statistics::startStamp = 0;

	M_INIT_CD(unsigned long, Vert);
	M_INIT_CD(unsigned long, Edge);
	M_INIT_CD(unsigned long, Tree);
	M_INIT_CD(unsigned long, Blos);
	M_INIT(unsigned long, MComp);
	M_INIT(unsigned long, Time);
	M_INIT(double, I);
	M_INIT(double, RI);

	Statistics::Statistics() {}
	Statistics::~Statistics() {}

	void Statistics::processCurrent() {
		Statistics::nMeasurements++;
		M_PROCESS_FIELD_CD(Vert);
		M_PROCESS_FIELD_CD(Edge);
		M_PROCESS_FIELD_CD(Tree);
		M_PROCESS_FIELD_CD(Blos);
		M_PROCESS_FIELD(MComp);
		M_PROCESS_FIELD(Time);
		M_PROCESS_FIELD(I);
		M_PROCESS_FIELD(RI);
	}

	void Statistics::reset() {
		Statistics::nMeasurements = 0;
		Statistics::processing = false;
		Statistics::paused = false;
		Statistics::timerRunning = false;

		M_RESET_CD(Vert);
		M_RESET_CD(Edge);
		M_RESET_CD(Tree);
		M_RESET_CD(Blos);
		M_RESET(MComp);
		M_RESET(Time);

		M_RESET(I);
		M_RESET(RI);
	}

	void Statistics::resetCurrent() {
		M_RESET_CUR_CD(Vert);
		M_RESET_CUR_CD(Edge);
		M_RESET_CUR_CD(Tree);
		M_RESET_CUR_CD(Blos);
		M_RESET_CUR(MComp);
		M_RESET_CUR(Time);
		M_RESET_CUR(I);
		M_RESET_CUR(RI);
	}

	void Statistics::startMeasure() {
		if (!Statistics::processing) {
			if (Statistics::paused) {
				Statistics::paused = false;
			} else {
				Statistics::resetCurrent();
			}
			Statistics::processing = true;
		}
	}

	void Statistics::pauseMeasure() {
		if (Statistics::processing) {
			Statistics::paused = true;
			Statistics::processing = false;
		}
	}

	void Statistics::stopMeasure() {
		if (Statistics::processing) {
			Statistics::pauseMeasure();
		}
		if (Statistics::paused) {
			Statistics::paused = false;
			Statistics::processing = false;
			if (Statistics::timerRunning) {
				Statistics::pauseTimer();
			}
			Statistics::processCurrent();
		}
	}

	void Statistics::startTimer() {
		Statistics::timerRunning = true;
		Statistics::startStamp = Time::currentTimeMillis();
	}

	void Statistics::pauseTimer() {
		if (Statistics::timerRunning) {
			Statistics::curTime += Time::currentTimeMillis() - Statistics::startStamp;
			Statistics::timerRunning = false;
		}
	}

	std::string Statistics::createReport() {
		std::stringstream ret;
		M_PRINT(ret, Time, "ms");
		M_PRINT_CD(ret, Vert, "");
		M_PRINT_CD(ret, Edge, "");
		M_PRINT_CD(ret, Tree, "");
		M_PRINT_CD(ret, Blos, "");
		M_PRINT(ret, MComp, "");
		M_PRINT(ret, I, "");
		M_PRINT(ret, RI, "");
		return ret.str();
	}

	M_INCREMENTER_CD(Vert);
	M_INCREMENTER_CD(Edge);
	M_INCREMENTER_CD(Tree);
	M_INCREMENTER_CD(Blos);

	void Statistics::processMComp(unsigned long comp) {
		if (Statistics::curMComp < comp) {
			Statistics::curMComp = comp;
		}
	}

	void Statistics::sort() {
		M_SORT_ALL_CD(Vert);
		M_SORT_ALL_CD(Edge);
		M_SORT_ALL_CD(Tree);
		M_SORT_ALL_CD(Blos);
		M_SORT_ALL(MComp);
		M_SORT_ALL(Time);
		M_SORT_ALL(I);
		M_SORT_ALL(RI);
	}

	M_GETTER_CD(unsigned long, Vert);
	M_GETTER_CD(unsigned long, Edge);
	M_GETTER_CD(unsigned long, Tree);
	M_GETTER_CD(unsigned long, Blos);
	M_GETTER(unsigned long, MComp);
	M_GETTER(unsigned long, Time);
	M_GETTER(double, I);
	M_GETTER(double, RI);

	M_SETTER(double, I);
	M_SETTER(double, RI);

	std::string Statistics::createCsvHeader() {
		std::stringstream ret;
#define M_APPEND(NAME) \
/**/ret << #NAME << " min, " \
/**/	<< #NAME << " max, " \
/**/	<< #NAME << " 5-percentile, " \
/**/	<< #NAME << " 95-percentile, " \
/**/	<< #NAME << " median, " \
/**/	<< #NAME << " avg";
#define M_APPEND_CD(NAME) \
/**/M_APPEND(NAME created) ret << ", "; M_APPEND(NAME deleted)
		M_APPEND(Computation time(ms)); ret << ", ";
		M_APPEND_CD(Vertices); ret << ", ";
		M_APPEND_CD(Edges); ret << ", ";
		M_APPEND_CD(Trees); ret << ", ";
		M_APPEND_CD(Blossoms); ret << ", ";
		M_APPEND(Max blossom complexity); ret << ", ";
		M_APPEND(I); ret << ", ";
		M_APPEND(RI);
#undef M_APPEND_CD
#undef M_APPEND
		return ret.str();
	}

	std::string Statistics::createCsvData() {
		Statistics::sort();
		std::stringstream ret;
#define M_APPEND(FIELD) \
/**/ret << Statistics::getMin ## FIELD() << ", " \
/**/	<< Statistics::getMax ## FIELD() << ", " \
/**/	<< Statistics::get05Percentile ## FIELD() << ", " \
/**/	<< Statistics::get95Percentile ## FIELD() << ", " \
/**/	<< Statistics::getMedian ## FIELD() << ", " \
/**/	<< Statistics::getAverage ## FIELD();
#define M_APPEND_CD(FIELD) \
/**/M_APPEND(FIELD ## Created) ret << ", "; M_APPEND(FIELD ## Deleted)
		M_APPEND(Time); ret << ", ";
		M_APPEND_CD(Vert); ret << ", ";
		M_APPEND_CD(Edge); ret << ", ";
		M_APPEND_CD(Tree); ret << ", ";
		M_APPEND_CD(Blos); ret << ", ";
		M_APPEND(MComp); ret << ", ";
		M_APPEND(I); ret << ", ";
		M_APPEND(RI);
#undef M_APPEND_CD
#undef M_APPEND
		return ret.str();
	}
}

#undef M_ULONG_MAX
#undef M_INFINITY

#undef M_UPDATE_MIN_MAX
#undef M_UPDATE_MIN_MAX_CD
#undef M_PROCESS_FIELD
#undef M_PROCESS_FIELD_CD
#undef M_INIT
#undef M_INIT_CD
#undef M_RESET
#undef M_RESET_CD
#undef M_RESET_CUR
#undef M_RESET_CUR_CD
#undef M_INCREMENTER
#undef M_INCREMENTER_CD
#undef M_GETTER
#undef M_GETTER_CD
#undef M_SETTER
#undef M_SETTER_CD
#undef M_SORT_ALL
#undef M_SORT_ALL_CD
#undef M_PRINT
#undef M_PRINT_CD