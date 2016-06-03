#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "conf.h"

#define MAXLINELENGTH 1024

char *trimwhitespace(char *str);

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
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

int conf_read(const std::string& filename, std::map < std::string, std::string >& my_map)
{
    using namespace std;
    char input[MAXLINELENGTH];
    FILE *fin;
    int len, ptr;
    bool found;

    if ((fin = fopen(filename.c_str(), "r")) == NULL)
    {
        cout << endl << "ERROR: Unable to open config file!" << endl;;
        return -1;
    }

    while (fgets(input, MAXLINELENGTH, fin))
    {
        if (input[0] == '#')
            continue;
        len = strlen(input);
        if (len < 2)
            continue;
        if (input[len-1] == '\n')
            input[len-1] = '\0';
        ptr = 0;
        found = false;
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
        	my_map[trimwhitespace(input)] = trimwhitespace(&input[ptr]);
        }
    }
    fclose(fin);
    return 0;
}

