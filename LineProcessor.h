#ifndef LINEPROCESSOR_H
#define LINEPROCESSOR_H

#include <string>

class LineProcessor
{
    public:
        LineProcessor();
        virtual ~LineProcessor();
        virtual bool Process(const std::string& line) = 0;
    protected:
    private:
};

#endif // LINEPROCESSOR_H
