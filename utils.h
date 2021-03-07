#ifndef UTILS_H_
#define UTILS_H_

#include <string>

#define BUFSIZE 4096
#define HISTORYFILENAME ".shell_history"

/*
	char *parse_memory(long int memory)
	------------------
	Convert a long int memory to a char array with appropriate suffix (KB/MB/GB)
*/
char* parse_memory(long int memory);

/*
	char *parse_time(long int time)
	------------------
	Convert a long int time to a char array with appropriate formatting (days, hours, minutes)
*/
char* parse_time(long int time);

// Store history in a file and fill it in a global variable
char* getHistoryFilename();

// Fetch Username, Hostname and OS Name from appropriate headers
std::string getUsername();
std::string getHostname();
std::string getOSName();

#endif // UTILS_H_