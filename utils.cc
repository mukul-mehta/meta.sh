#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>

#include "utils.h"

char *parse_memory(long int memory)
{
	char *response = (char *)malloc(1024 * sizeof(char));
	if (memory <= 1024)
	{
		int mem = (int)memory;
		sprintf(response, "%dB", mem);
	}
	else if (memory <= 1024 * 1024)
	{
		int mem = (int)(memory / 1024.0);
		sprintf(response, "%dKB", mem);
	}
	else
	{
		int mem = (int)(memory / (1024.0 * 1024.0));
		sprintf(response, "%dMB", mem);
	}
	return response;
}

char *parse_time(long int time)
{
	int days = time / (24 * 3600);
	int hours = (time - (days * 24 * 3600)) / 3600;
	int minutes = (time - hours * 3600) / 60;

	char *response = (char *)malloc(1024 * sizeof(char));

	if (hours == 0)
	{
		sprintf(response, "%d minutes", minutes);
	}
	else if (days == 0)
	{
		if (hours == 1)
			sprintf(response, "%d hour, %d minutes", hours, minutes);
		else
			sprintf(response, "%d hours, %d minutes", hours, minutes);
	}
	else
	{
		if (days == 1)
			sprintf(response, "%d day, %d hours, %d minutes", days, hours, minutes);
		else
			sprintf(response, "%d days, %d hours, %d minutes", days, hours, minutes);
	}
	return response;
}

string getUsername()
{
	char *username = (char *)malloc(BUFSIZE * sizeof(char));
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	if (!pw)
	{
		username = (char *)"";
	}
	username = pw->pw_name;

	return string(username);
}

string getHostname()
{
	char *hostname = (char *)malloc(BUFSIZE * sizeof(char));
	int ret = gethostname(hostname, BUFSIZE);
	if (ret < 0)
	{
		hostname = (char *)"";
	}
	return string(hostname);
}

string getOSName()
{
	ifstream infile("/etc/os-release");

	if (infile.good())
	{
		string temp;
		getline(infile, temp);
		string OSName = temp.substr(temp.find("=") + 2, temp.size() - 1);
		OSName.pop_back();
		return OSName;
	}
	return "Unknown Linux Distribution";
}
