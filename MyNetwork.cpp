#include "MyNetwork.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <regex>

#ifdef WIN32
#include <winsock2.h>
#endif // WIN32

#include "happyhttp.h"
#include "MyUtils.h"

void MyNetwork::OnBegin( const happyhttp::Response* r) {
    dataReceived.clear();
}

void MyNetwork::OnData( const happyhttp::Response* r, const unsigned char* data, int n ) {
    dataReceived += std::string((const char*)data, n);
}

void MyNetwork::OnComplete( const happyhttp::Response* r) {
}

void MyNetwork::OnBeginPtr( const happyhttp::Response* r, void* userdata ) {
    ((MyNetwork*)userdata)->OnBegin(r);
}

void MyNetwork::OnDataPtr( const happyhttp::Response* r, void* userdata, const unsigned char* data, int n ) {
    ((MyNetwork*)userdata)->OnData(r, data, n);
}

void MyNetwork::OnCompletePtr( const happyhttp::Response* r, void* userdata ) {
    ((MyNetwork*)userdata)->OnComplete(r);
}

void MyNetwork::ParseUrl(const std::string& url, std::string& head, std::string& tail) {
    size_t protDelim = 0;
    if((protDelim = url.find("://")) != std::string::npos) {
            protDelim += 3;
            if(protDelim > url.length()) {
                head = tail = "";
                std::cout << std::endl << "ERROR: Wrong url: " << url << std::endl;
                return;
            }
    }
    else {
        protDelim = 0;
    }
    tail.clear();
    size_t delim = url.find('/', protDelim);
    if(delim != std::string::npos && delim > protDelim) {
        head = url.substr(protDelim, delim - protDelim);
        tail = url.substr(delim);
    }
    else {
        head = url.substr(protDelim);
    }
}

std::string MyNetwork::GetExtIp() {
    std::string head, tail;
    ParseUrl(conf["EXT_IP_URL"], head, tail);
    if(head.empty()) {
        return "";
    }
    happyhttp::Connection conn( head.c_str(), 80 );
    conn.setcallbacks( OnBeginPtr, OnDataPtr, OnCompletePtr, (void*)this);

    conn.request( "GET", tail.c_str(), 0, 0, 0 );

    while( conn.outstanding() )
        conn.pump();
    return MyUtils::trim(dataReceived);
}

std::string MyNetwork::GetTestId(const std::map<std::string, std::string>& _params) {
    using namespace std;
    map<string, string> params = _params;
    const char* headers[] = {
        "Content-type", "application/json",
        0
    };

    string body = "{\"key\":\"";
    body += conf["RESULTS_KEY"];
    body += "\", \"download\":\"";
    body += params["DOWNLOAD_SPEED"];
    body += "\", \"upload\":\"";
    body += params["UPLOAD_SPEED"];
    body += "\", \"latency\":\"";
    body += params["LATENCY"];
    body += "\"}";
    string head, tail;
    ParseUrl(conf["RESULTS_URL"], head, tail);
    if(head.empty()) {
        return "";
    }
    happyhttp::Connection conn( head.c_str(), 80);
    conn.setcallbacks( OnBeginPtr, OnDataPtr, OnCompletePtr, (void*)this );
    conn.request( "POST",
                  tail.c_str(),
                  headers,
                  (const unsigned char*)body.c_str(),
                  body.length() );

    while( conn.outstanding() )
        conn.pump();
    regex re_testid(R"(\{\s*\'test_id\'\s*\:\s*\'(.*)\')");
    smatch m;
    if(regex_search(dataReceived, m, re_testid) && m.size() > 1) {
         return m.str(1);
    }
    return "";
}

int MyNetwork::DisplayResults(const std::string& testId) {
	using namespace std;
	string fullUrl = conf["RESULTS_SHOW_URL"] + "?test_id=" + testId;
	std::wstring fullUrlW = std::wstring(fullUrl.begin(), fullUrl.end());
	ShellExecute(0, 0, fullUrlW.c_str(), 0, 0, SW_SHOW);
	return 0;
}

MyNetwork::MyNetwork(const std::map<std::string, std::string>& config) {
    this->conf = config;
#ifdef WIN32
    WSAData wsaData;
    int code = WSAStartup(MAKEWORD(1, 1), &wsaData);
    if( code != 0 ) {
        std::cout << std::endl << "ERROR: Failed to start WSA:" <<  code << std::endl;
    }
#endif //WIN32
}

MyNetwork::~MyNetwork() {
#ifdef WIN32
    WSACleanup();
#endif // WIN32
}
