#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "base64.h"
#include "MyUtils.h"

#include "conf.h"

#define MAXLINELENGTH 1024

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

void process_line(char* input, std::map < std::string, std::string >& my_map)
{
	if (input[0] == '#')
		return;
	int len = strlen(input);
	if (len < 2)
		return;
	if (input[len - 1] == '\n')
		input[len - 1] = '\0';
	int ptr = 0;
	bool found = false;
	while (ptr < len)
	{
		if (input[ptr] == '=')
		{
			found = true;
			input[ptr++] = '\0';
			break;
		}
		ptr++;
	}
	if (found)
	{
		std::string key = input;
		std::string val = &input[ptr];
		my_map[MyUtils::trim(key)] = MyUtils::trim(val);
	}
}

int conf_read(const std::string& filename, std::map < std::string, std::string >& my_map)
{
    using namespace std;
    char input[MAXLINELENGTH];
    FILE *fin;

    if ((fin = fopen(filename.c_str(), "r")) == NULL)
    {
        cout << endl << "ERROR: Unable to open config file!" << endl;;
        return -1;
    }

    while (fgets(input, MAXLINELENGTH, fin))
    {
		process_line(input, my_map);
    }
    fclose(fin);
    return 0;
}

int conf_read_base64(const std::string& base64str, std::map<std::string, std::string>& my_map)
{
	std::string conf = base64_decode(base64str);
	std::stringstream sstr;
	sstr << conf;
	std::string line;
	char input[MAXLINELENGTH];
	while (sstr.getline(input, MAXLINELENGTH))
	{
		process_line(input, my_map);
	}
	return 0;
}

