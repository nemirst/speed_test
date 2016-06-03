#ifndef PROGRESS_H
#define PROGRESS_H

#include <map>
#include <string>

class Progress {
public:
	enum ProgressStage {
		PING,
		DOWN_CON,
		DOWN_CLIENT_CON,
		DOWN_TEST,
		DOWN_END,
		UP_CON,
		UP_CLIENT_CON,
		UP_TEST,
		UP_END,
		DB,
		RESULTS
	};
private:
	struct Value_t {
		int seconds;  // estimated seconds for this progress stage
		float percentPrevEnd;  // progress percentage of previous progress stage end
		float percentEnd;  // progress percentage of current progress stage end
		int totalParts;  // total parts in this progress stage
		Value_t(int s = 0, float pp = 0.0f, float p = 0.0f, int t = 1) :
			seconds(s), percentPrevEnd(pp), percentEnd(p), totalParts(t) {
		}
	};
	std::map<std::string, std::string> conf;
	std::map<ProgressStage, Value_t> est;
	void FillProgressEstimates();
public:
	Progress(const std::map<std::string, std::string>& c) {
		conf = c;
		FillProgressEstimates();
	}

	static void ShowProgress(int progress);
	void ShowProgress(ProgressStage stage, int partDone) const;
};

#endif // PROGRESS_H