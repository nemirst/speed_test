#include <iostream>
#include <regex>

#include "IperfProc.h"

bool IperfProc::Process(const std::string& line) {
    using namespace std;
    //cout << line << endl;

    regex re_end("iperf Done.");
    if(regex_search(line, re_end)) {
        success = true;
        return false;
    }

    regex re_fail("iperf: error");
    if(regex_search(line, re_fail)) {
        cout << endl << "ERROR: iPerf command failed" << endl;
        return false;
    }

	regex re_local(R"(\s+local\s+)");
	if (regex_search(line, re_local)) {
		if (mode == DOWNLOAD) {
			p.ShowProgress(Progress::DOWN_CLIENT_CON, ++down_clients);
		}
		else if (mode == UPLOAD) {
			p.ShowProgress(Progress::UP_CLIENT_CON, ++up_clients);
		}
		return true;
	}

	smatch m;
	if (mode == DOWNLOAD) {
		regex re_sum_rec(R"(\[\s*SUM\s*\].*\s+(.+)\s+[a-zA-Z]+\/sec.+receiver)");
		if (regex_search(line, m, re_sum_rec) && m.size() > 1) {
			this->sum = (float)atof(m.str(1).c_str());
			p.ShowProgress(Progress::DOWN_END, 0);
			return true;
		}
	}
	if(mode == UPLOAD) {
		regex re_sum_send(R"(\[\s*SUM\s*\].*\s+(.+)\s+[a-zA-Z]+\/sec.+sender)");
		if (regex_search(line, m, re_sum_send) && m.size() > 1) {
			this->sum = (float)atof(m.str(1).c_str());
			p.ShowProgress(Progress::UP_END, 0);
			return true;
		};
	}

	regex re_sum(R"(\[\s*SUM\s*\])");
	if (regex_search(line, re_sum)) {
		if (mode == DOWNLOAD) {
			p.ShowProgress(Progress::DOWN_TEST, ++down_seconds);
		}
		else if (mode == UPLOAD) {
			p.ShowProgress(Progress::UP_TEST, ++up_seconds);
		}
		return true;
	}

    return true;
}
