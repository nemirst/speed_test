#ifndef MYNETWORK_H
#define MYNETWORK_H
#include "happyhttp.h"

class MyNetwork
{
    public:
        MyNetwork(const std::map<std::string, std::string>& conf);
        virtual ~MyNetwork();
        std::string GetExtIp();
        std::string GetTestId(const std::map<std::string, std::string>& params);
		int DisplayResults(const std::string& testId);
    protected:
    private:
        std::string dataReceived;
        std::map<std::string, std::string> conf;
        void OnBegin( const happyhttp::Response* r);
        void OnData( const happyhttp::Response* r, const unsigned char* data, int n );
        void OnComplete( const happyhttp::Response* r);
        static void OnBeginPtr( const happyhttp::Response* r, void* userdata);
        static void OnDataPtr( const happyhttp::Response* r, void* userdata, const unsigned char* data, int n );
        static void OnCompletePtr( const happyhttp::Response* r, void* userdata );
		static void ParseUrl(const std::string& url, std::string& head, std::string& tail);
};

#endif // MYNETWORK_H
