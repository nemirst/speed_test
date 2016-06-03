// ConfigSigner.cpp : Defines the entry point for the console application.
//

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <md5.h>

void PrintUsage()
{
	using namespace std;
	cout << "Usage:" << endl << "ConfigSigner <file to sign>" << endl;
}

std::string ReadContents(const std::string& file_name)
{
	using namespace std;
	ifstream in(file_name);
	if (!in.is_open())
	{
		cerr << "Error: " << strerror(errno) << endl;
		return "";
	}
	stringstream buffer;
	buffer << in.rdbuf();
	in.close();
	return buffer.str();
}

int SaveHash(const std::string& orig_file_name, const std::string& hash_value)
{
	using namespace std;
	ofstream out(orig_file_name + ".sig");
	if (!out.is_open())
	{
		cerr << "Error: " << strerror(errno) << endl;
		return -1;
	}
	out << hash_value;
	out.close();
	return 0;
}

int main(int argc, char* argv[])
{
	using namespace std;
	string secret = "49b8d727d50876446b4da9659aa8cde9";
	if (argc != 2)
	{
		PrintUsage();
		return -1;
	}
	string file_name = argv[1];
	string s = ReadContents(file_name);
	if (s.length() > 0)
	{
		s += secret;
		s = md5(s);
		return SaveHash(file_name, s);
	}
	return 0;
}

