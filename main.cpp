#include <iostream>
#include <fstream>
#include <regex>
#include <iomanip>
#include <sstream>
#include <tuple>

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <conio.h>

#include <md5.h>

#include "my_sleep.h"
#include "conf.h"
#include "IperfProc.h"
#include "MyNetwork.h"
#include "DbHandling.h"
#include "Progress.h"


std::map<std::string, std::string> conf;

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}

// Calls process in background and checks it's output.
int StreamCommand(const std::string& command, LineProcessor& proc,
                  const std::string& tempFileName, bool redirect = true) {
    using namespace std;
    const int MAX_RETRIES_OPEN = 50;
    const int REFRESH_MS = 100;
    const int MAX_RETRIES_READ = 5000;

	std::remove(tempFileName.c_str());
    string cmd_line = "START /b " + command;
    if(redirect) {
        cmd_line += " > " + tempFileName;
    }
    if(system(cmd_line.c_str()) != 0) {
        cout << endl << "ERROR: Failed to call command" << endl;
        return -1;
    }
    // Wait until file is created by system call.
    ifstream stream_results(tempFileName);
    int retries = MAX_RETRIES_OPEN;
    while(retries-- && !stream_results.is_open()) {
        delay(REFRESH_MS);
        stream_results.open(tempFileName);
    }
    if(!stream_results.is_open()) {
        cout << endl << "ERROR: Failed to execute command & get output" << endl;
        return -1;
    }

    retries = MAX_RETRIES_READ;
    while(retries) {
        stream_results.clear();
        string line;
        if(getline(stream_results, line)) {
            retries = MAX_RETRIES_READ;
            if(!proc.Process(line)) {
                break;
            }
        } else {
            delay(REFRESH_MS);
            retries--;
        }
    }
    stream_results.close();
    if(retries == 0) {
        cout << endl << "ERROR: Command did not return expected results" << endl;
		std::remove(tempFileName.c_str());
        return -1;
    }
	std::remove(tempFileName.c_str());
    return 0;
}

int HandlePing(int& latency) {
    using namespace std;
    string ping_cmd_line = conf["PING_CMD"] +
        " | FIND \"Average\" > ping_results.txt";
    if(system(ping_cmd_line.c_str()) != 0) {
        cout << endl <<  "ERROR: Failed to call ping" << endl;
        return -1;
    }
    ifstream ping_results("ping_results.txt");
    if(!ping_results.is_open()) {
        cout << endl << "ERROR: Failed to retrieve ping results from file" << endl;
		std::remove("ping_results.txt");
        return -1;
    }
    string results;
    getline(ping_results, results);
    smatch match;
    regex re("Average = (\\d+)ms");
    if(!regex_search(results, match, re) || match.size() < 2) {
        cout << endl << "ERROR: Failed to parse ping result" << endl;
		std::remove("ping_results.txt");
        return -1;
    }
    latency = atoi(match.str(1).c_str());
    ping_results.close();
	std::remove("ping_results.txt");
    return 0;
}

int HandleIperf(const Progress& p, int& downloadSpeed, int& uploadSpeed, std::string& downloadTime, std::string& uploadTime,
	std::string& downloadDuration, std::string& uploadDuration) {
    using namespace std;
    IperfProc procDown(p, IperfProc::DOWNLOAD);
    IperfProc procUp(p, IperfProc::UPLOAD);

	p.ShowProgress(Progress::DOWN_CON, 0);
	downloadTime = currentDateTime();
	clock_t begin = clock();
    StreamCommand(conf["IPERF_DOWN_CMD"], procDown, "download_stream.txt", false);
    if(procDown.getSuccess()) {
        downloadSpeed = (int)procDown.getSum();
    }
    else {
        cout << endl << "ERROR: Failed to get average download speed from iPerf3" << endl;
        return -1;
    }
	clock_t end = clock();
	stringstream ss;
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	ss << fixed << setprecision(2) << elapsed_secs;
	downloadDuration = ss.str();
	ss.str(std::string());
	ss.clear();

	p.ShowProgress(Progress::UP_CON, 0);
	uploadTime = currentDateTime();
	begin = clock();
    StreamCommand(conf["IPERF_UP_CMD"], procUp, "upload_stream.txt", false);
    if(procUp.getSuccess()) {
        uploadSpeed = (int)procUp.getSum();
    }
    else {
        cout << endl << "ERROR: Failed to get average upload speed from iPerf3" << endl;
        return -1;
    }
	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	ss << fixed << setprecision(2) << elapsed_secs;
	uploadDuration = ss.str();
	ss.str(std::string());
	ss.clear();
	p.ShowProgress(Progress::UP_END, 1);
    return 0;
}

int HandleDb(const std::map<std::string, std::string>& params) {
    using namespace std;
	DbHandling dbObj(conf);
	dbObj.Store(params);
    return 0;
}

int HandleExtIp(MyNetwork& n, std::string& extIp) {
    extIp = n.GetExtIp();
    return extIp.length() ? 0 : -1;
}

int HandleResults(const Progress& p, MyNetwork& n, const std::map<std::string, std::string>& params) {
	using namespace std;
    string testId = n.GetTestId(params);
	if (testId.size() == 0) {
		return -1;
	}
	p.ShowProgress(Progress::RESULTS, 1);
	return n.DisplayResults(testId);
}

int main() {
    using namespace std;

	string config_str = ReadContents("config.txt");
	if (config_str.length() == 0)
	{
		cout << endl << "ERROR: Failed to read configuration file" << endl;
		return -1;
	}
	string config_sig = ReadContents("config.txt.sig");
	if (config_sig.length() == 0)
	{
		cout << endl << "ERROR: Failed to read signature of configuration file" << endl;
		fflush(stdin);
		_getch();
		return -1;
	}

	string secret = "49b8d727d50876446b4da9659aa8cde9";
	string real_sig = md5(config_str + secret);
	if (config_sig != real_sig)
	{
		cout << endl << "ERROR: wrong signature " << real_sig <<
			" for configuration file. Changed contents?" << endl;
		fflush(stdin);
		_getch();
		return -1;
	}

	if (conf_read("config.txt", conf) != 0) {
		cout << endl << "ERROR: Failed to read configuration file" << endl;
		fflush(stdin);
		_getch();
		return -1;
	}
	
	cout << string(70, '=') << endl;
	cout << "Singtel Speed Testing for 10G Network" << endl << endl;
	cout << "Powered by iPerf3" << endl;
	cout << string(70, '=') << endl;
	cin.clear();
	cout << "Press Any Key to Continue....." << endl;
	fflush(stdin);
	_getch();
	cout << endl << "Testing in Progress" << endl;

	Progress::ShowProgress(0);

	Progress p(conf);

    MyNetwork net(conf);

    map<string, string> params;
    int ret = 0;
    int latency = 0;
    if((ret = HandlePing(latency)) != 0) {
		fflush(stdin);
		_getch();
        return ret;
    }
	p.ShowProgress(Progress::PING, 1);

    int downloadSpeed = 0, uploadSpeed = 0;
	string downloadTime, uploadTime, downloadDuration, uploadDuration;
    if((ret = HandleIperf(p, downloadSpeed, uploadSpeed, downloadTime, uploadTime, downloadDuration, uploadDuration)) != 0) {
		fflush(stdin);
		_getch();
        return ret;
    }

    string extIp;
    if((ret = HandleExtIp(net, extIp)) != 0) {
		fflush(stdin);
		_getch();
        return ret;
    }

    params["EXT_IP"] = extIp;
	params["LATENCY"] = to_string(latency);
	params["DOWNLOAD_TIME"] = downloadTime;
	params["DOWNLOAD_DURATION"] = downloadDuration;
	params["DOWNLOAD_SPEED"] = to_string(downloadSpeed);
	params["UPLOAD_TIME"] = uploadTime;
	params["UPLOAD_DURATION"] = uploadDuration;
	params["UPLOAD_SPEED"] = to_string(uploadSpeed);

	//cout << "EXT_IP: '" << params["EXT_IP"] << "'" << endl;
	//cout << "LATENCY: " << params["LATENCY"] << endl;
	//cout << "DOWNLOAD_TIME: " << params["DOWNLOAD_TIME"] << endl;
	//cout << "DOWNLOAD_DURATION: " << params["DOWNLOAD_DURATION"] << endl;
	//cout << "DOWNLOAD_SPEED: " << params["DOWNLOAD_SPEED"] << endl;
	//cout << "UPLOAD_TIME: " << params["UPLOAD_TIME"] << endl;
	//cout << "UPLOAD_DURATION: " << params["UPLOAD_DURATION"] << endl;
	//cout << "UPLOAD_SPEED: " << params["UPLOAD_SPEED"] << endl;

	p.ShowProgress(Progress::DB, 0);
    if((ret = HandleDb(params)) != 0) {
		// Ignore DB errors - only for logging purposes.
        //return ret;
    }
	p.ShowProgress(Progress::RESULTS, 0);
    if((ret = HandleResults(p, net, params)) != 0) {
		fflush(stdin);
		_getch();
        return ret;
    }
	Progress::ShowProgress(100);
	cout << endl << "Completed" << endl;
	fflush(stdin);
	_getch();
    return 0;
}
