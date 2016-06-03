#ifndef DBHANDLING_H
#define DBHANDLING_H


class DbHandling
{
    public:
		DbHandling(const std::map<std::string, std::string>& c);
        virtual ~DbHandling();
        int Store(const std::map<std::string, std::string>& params);
    protected:
    private:
		std::map<std::string, std::string> conf;
};

#endif // DBHANDLING_H
