#include <iostream>
#include <string>
#include <map>
#include <regex>

#include "Progress.h"

void Progress::FillProgressEstimates() {
	using namespace std;
	est[PING] = Value_t(4);
	est[DOWN_CON] = Value_t(3);
	est[DOWN_CLIENT_CON] = Value_t(); // Calculate
	est[DOWN_TEST] = Value_t(); // Calculate
	est[DOWN_END] = Value_t(3);
	est[UP_CON] = Value_t(3);
	est[UP_CLIENT_CON] = Value_t(); // Calculate
	est[UP_TEST] = Value_t(); // Calculate
	est[UP_END] = Value_t(3);
	est[DB] = Value_t(3);
	est[RESULTS] = Value_t(1);
	string down_cmd = conf["IPERF_DOWN_CMD"];
	string up_cmd = conf["IPERF_UP_CMD"];
	regex re_t(R"(\s+\-t\s+(\d+))");
	regex re_P(R"(\s+\-P\s+(\d+))");
	smatch m;
	int down_t = 10, down_P = 10, up_t = 10, up_P = 10;
	if (regex_search(down_cmd, m, re_t) && m.size() > 1) {
		down_t = atoi(m.str(1).c_str());
	}
	est[DOWN_TEST] = Value_t(down_t, 0, 0, down_t);
	if (regex_search(down_cmd, m, re_P) && m.size() > 1) {
		down_P = atoi(m.str(1).c_str());
	}
	est[DOWN_CLIENT_CON] = Value_t((int)(down_P * 0.5f), 0, 0, down_P);  // set half second for every parallel connection.
	if (regex_search(up_cmd, m, re_t) && m.size() > 1) {
		up_t = atoi(m.str(1).c_str());
	}
	est[UP_TEST] = Value_t(up_t, 0, 0, up_t);
	if (regex_search(up_cmd, m, re_P) && m.size() > 1) {
		up_P = atoi(m.str(1).c_str());
	}
	est[UP_CLIENT_CON] = Value_t((int)(up_P * 0.5f), 0, 0, up_P);  // set half second for every parallel connection.

	int total_seconds = 0;
	for (const auto& val : est) {
		total_seconds += val.second.seconds;
	}
	int cumulative_sec = 0;
	for (auto& val : est) {
		val.second.percentPrevEnd = (float)cumulative_sec / (float)total_seconds * 100.0f;
		cumulative_sec += val.second.seconds;
		val.second.percentEnd = (float)cumulative_sec / (float)total_seconds * 100.0f;
	}
}

void Progress::ShowProgress(int progress) {
	int barWidth = 70;
	std::cout << "[";
	int pos = (int)(barWidth * progress * 0.01f);
	if (progress >= 100) {
		pos = barWidth;
	}
	int lenMiddle = std::to_string(progress).length() + 1; // 2, 3 or 4  ##########
	int middleStartPos = barWidth / 2 - lenMiddle / 2;   // barwidth = 10 4, 4, 3
	for (int i = 0; i < barWidth; ++i) {
		if (i == middleStartPos) {
			std::cout << progress << "%";
			i += lenMiddle - 1;
		}
		else
			if (i < pos) {
				std::cout << "#";
			}
			else std::cout << " ";
	}
	std::cout << "]\r";
	std::cout.flush();
}

void Progress::ShowProgress(ProgressStage stage, int partDone) const {
	Value_t val = est.at(stage);
	float intervalNow = val.percentEnd - val.percentPrevEnd;
	if (partDone > val.totalParts) {
		partDone = val.totalParts;
	}
	float doneNow = (float)(partDone) / (float)(val.totalParts);
	int progress = (int)(val.percentPrevEnd + doneNow * intervalNow);
	ShowProgress(progress);
}