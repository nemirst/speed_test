#ifndef IPERFPROC_H
#define IPERFPROC_H

#include <string>
#include "LineProcessor.h"
#include "Progress.h"

class IperfProc : public LineProcessor {
public:
	enum Mode { DOWNLOAD, UPLOAD };
private:
	Progress p;
	float sum;
    bool success;
	Mode mode;
	int down_clients;
	int up_clients;
	int down_seconds;
	int up_seconds;
public:
	IperfProc(Progress p_, Mode m) : mode(m), sum(0.0f), success(false), p(p_),
	down_clients(0), up_clients(0), down_seconds(0), up_seconds(0){
    }
    virtual ~IperfProc() {
    }
    bool getSuccess() {
        return success;
    }
    float getSum() {
        return sum;
    }
    virtual bool Process(const std::string& line);
};

#endif // IPERFPROC_H
