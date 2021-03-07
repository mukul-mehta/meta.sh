#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

using namespace std;

char* parse_memory(long int memory) {
    char* response = (char*)malloc(1024 * sizeof(char));
    if (memory <= 1024) {
        int mem = (int)memory;
        sprintf(response, "%dB", mem);
    } else if (memory <= 1024 * 1024) {
        int mem = (int)(memory / 1024.0);
        sprintf(response, "%dKB", mem);
    } else {
        int mem = (int)(memory / (1024.0 * 1024.0));
        sprintf(response, "%dMB", mem);
    }
    return response;
}

char* parse_time(long int time) {
    int days = time / (24 * 3600);
    int hours = (time - (days * 24 * 3600)) / 3600;
    int minutes = (time - hours * 3600) / 60;

    char* response = (char*)malloc(1024 * sizeof(char));

    if (hours == 0) {
        sprintf(response, "%d minutes", minutes);
    } else if (days == 0) {
        if (hours == 1)
            sprintf(response, "%d hour, %d minutes", hours, minutes);
        else
            sprintf(response, "%d hours, %d minutes", hours, minutes);
    } else {
        if (days == 1)
            sprintf(response, "%d day, %d hours, %d minutes", days, hours, minutes);
        else
            sprintf(response, "%d days, %d hours, %d minutes", days, hours, minutes);
    }
    return response;
}

char* getHistoryFilename() {
    char* histfile = (char*)malloc(BUFSIZE * sizeof(char));
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (!pw) {
        histfile = NULL;
    }
    histfile = pw->pw_dir;

    char* fullFilePath = (char*)malloc(BUFSIZE * sizeof(char));
    sprintf(fullFilePath, "%s/%s", histfile, HISTORYFILENAME);
    return fullFilePath;
}

const char* getUsername() {
    char* username = (char*)malloc(BUFSIZE * sizeof(char));
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (!pw) {
        username = (char*)"";
    }
    username = pw->pw_name;

    return username;
}

const char* getHostname() {
    char* hostname = (char*)malloc(BUFSIZE * sizeof(char));
    int ret = gethostname(hostname, BUFSIZE);
    if (ret < 0) {
        hostname = (char*)"";
    }
    return hostname;
}

const char* getOSName() {
    FILE* infile = fopen("/etc/os-release", "r");
    char* line = NULL;
    size_t len = 0;

    if (infile) {
        getline(&line, &len, infile);
        char* temp = (char*)malloc(BUFSIZE * sizeof(char));

        strcpy(temp, line);
        strtok(temp, "=");

        char* result = (char*)malloc(BUFSIZE * sizeof(char));
        strncpy(result, &line[strlen(temp) + 2], strlen(line) - strlen(temp) - 4);
        return result;
    }
    return "Unknown Linux Distribution";
}
