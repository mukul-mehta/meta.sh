#include <pwd.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>

#include <algorithm>

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "builtins.h"
#include "utils.h"

using namespace std;

char __CWD[BUFSIZE];

int metash_exit(unused vector<string> tokens) { exit(EXIT_SUCCESS); }

int metash_help(vector<string> tokens) {
    printf("%s+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s\n",
           PURPLE, NORM);
    printf("%s++++++%s /\\        %sHi there. There are \033[4mfive%s %sbuiltin commands%s     /\\ "
           " %s++++++%s\n",
           PURPLE, NORM, CYAN, NORM, CYAN, NORM, PURPLE, NORM);
    printf("%s++++++%s cd%s:       Changes working directory to the one specified    %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s pwd%s:      Shows current working directory                   %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s help%s:     Shows this help text                              %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s exit%s:     Cleanly exits the shell                           %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s fetch%s:    Show system information                           %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s history%s:  Show all commands executed on the shell           %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s getenv%s:   Set an environment variable to specified value    %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s setenv%s:   Fetch the value of the given environment variable %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s unsetenv%s: Unset the given environment variable              %s++++++%s\n",
           PURPLE, YELLOW, NORM, PURPLE, NORM);
    printf("%s++++++%s Anything else is considered as an executable, and should    %s++++++%s\n",
           PURPLE, BLUE, PURPLE, NORM);
    printf("%s++++++%s be present in your PATH. \033[1;3;34mEnjoy!%s                             "
           "%s++++++%s\n",
           PURPLE, BLUE, NORM, PURPLE, NORM);
    printf("%s+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s\n",
           PURPLE, NORM);

    return 0;
}

int metash_pwd(vector<string> tokens) {
    size_t num_tokens = tokens.size();
    if (num_tokens >= 2) {
        printf("pwd: too many arguments\n");
        return -1;
    }

    char* current_directory = (char*)malloc(BUFSIZE * sizeof(char));
    string pwd = getcwd(current_directory, BUFSIZE);
    if (!pwd.empty()) {
        printf("%s\n", pwd.c_str());
        return 0;
    }
    perror("Error in fetching current working directory");
    return -1;
}

int metash_cd(vector<string> tokens) {
    size_t num_tokens = tokens.size();
    if (num_tokens >= 3) {
        printf("cd: too many arguments\n");
        return -1;
    }

    string new_directory_rel;
    char* new_directory = (char*)malloc(BUFSIZE * sizeof(char));
    // If no argument is passed, assume cd to home directory of user. Fetch home directory path
    if (num_tokens == 1) {
        uid_t uid = getuid();
        struct passwd* pw = getpwuid(uid);
        new_directory_rel =
            pw->pw_dir; // Stores the home direcoty path of the user executing the program
    } else {
        new_directory_rel = tokens[1];
    }
    // In case of links, or if a relative path is specified, generate the real path
    char* temp = realpath(new_directory_rel.c_str(), new_directory);
    if (!temp)
        perror("Couldn't get realpath");

    int ret = chdir(new_directory);
    if (ret < 0) {
        printf("Error in changing current directory. pwd not changed");
        return -1;
    }
    strcpy(__CWD, new_directory);
    return 0;
}

int metash_fetch(unused vector<string> tokens) {
    // Fetch username, hostname and OS Name
    unused string username = getUsername();
    unused string hostname = getHostname();
    unused string OSname = getOSName();

    string kernel_name, kernel_version, machine;

    long uptime = -1;
    long total_memory = -1;
    long free_memory = -1;
    long consumed_memory = -1;

    struct utsname uts;
    // The `uname` call populates the uts struct with system information. Also implemented as the `uname` executable
    if (uname(&uts) < 0) {
        perror("uname error");
    } else {
        kernel_name = uts.sysname;
        kernel_version = uts.release;
        machine = uts.machine;
    }

    struct sysinfo info;
    // The `sysinfo` call populates the `sysinfo` struct. Fills with uptime and memory usage information
    if (sysinfo(&info) < 0)
        perror("sysinfo() error");
    else {
        total_memory = info.totalram;
        free_memory = info.freeram;
        consumed_memory = total_memory - free_memory;
        uptime = info.uptime;
    }

    char* user_host_string = (char*)malloc(BUFSIZE * sizeof(char));
    sprintf(user_host_string, "%s%s@%s%s\n", BLUE, username.c_str(), hostname.c_str(), NORM);
    size_t len = username.size() + hostname.size();

    printf(user_host_string);
    for (size_t i = 0; i <= len; i++)
        printf("-");
    printf("\n");

    printf("%sOS%s:       %s\n", BLUE, NORM, OSname.c_str());
    printf("%sKernel%s:   %s %s\n", BLUE, NORM, kernel_name.c_str(), kernel_version.c_str());
    printf("%sPlatform%s: %s\n", BLUE, NORM, machine.c_str());
    printf("%sMemory%s:   %s / %s\n", BLUE, NORM, parse_memory(consumed_memory),
           parse_memory(total_memory));
    printf("%sUptime%s:   %s\n", BLUE, NORM, parse_time(uptime));
    printf("%sShell%s:    %s %s\n", BLUE, NORM, SHELL, VERSION);
    printf("%sAuthors%s:  %s\n", BLUE, NORM, AUTHORS);

    return 0;
}

int metash_execute(vector<string> tokens) {
    /*
		Check if there is a < token in the token vector. If found, the next token of < will be the filename
		Remove this token and the filename since the execvp call needs only the arguments passed
		Open the file and use the `dup2` system call to set appropriate file descriptor entry
	*/

    vector<string>::iterator inpIter = find(tokens.begin(), tokens.end(), "<");
    if (inpIter != tokens.end()) {
        int index = inpIter - tokens.begin();
        int inputFD = open(tokens[index + 1].c_str(), O_RDONLY);
        int status = dup2(inputFD, READ_FLAGS);
        if (status == -1)
            perror("dup2() failed");

        tokens.erase(inpIter, inpIter + 2);
    }

    // Same as above, check for existence of > token and use the `dup2` call with the output file
    vector<string>::iterator outIter = find(tokens.begin(), tokens.end(), ">");
    if (outIter != tokens.end()) {
        int index = outIter - tokens.begin();
        int outputFD = open(tokens[index + 1].c_str(), WRITE_FLAGS, 0644);
        int status = dup2(outputFD, STDOUT_FILENO);
        if (status == -1)
            perror("dup2() failed");

        tokens.erase(outIter, outIter + 2);
    }

    size_t num_tokens = tokens.size();
    // `execvp` requires a char array with the last element set to NULL
    char* args[num_tokens + 1];
    for (size_t i = 0; i < tokens.size(); i++)
        args[i] = (char*)(tokens[i].c_str());

    args[num_tokens] = NULL;

    int ret = execvp(tokens[0].c_str(), args);
    if (ret == -1) {
        printf("execvp() failed: Command not found: %s\n", tokens[0].c_str());
    }
    exit(EXIT_FAILURE);
}

int metash_history(unused vector<string> tokens) {
    HIST_ENTRY** hist_list = history_list();

    if (!hist_list)
        return 1;

    for (int i = 0; i < history_length; i++)
        printf("%s%d%s: %s\n", RED, i + history_base, NORM, hist_list[i]->line);

    return 0;
}

int metash_setenv(vector<string> tokens) {
    size_t num_tokens = tokens.size();
    if (num_tokens >= 4) {
        printf("setenv: too many arguments\n");
        return -1;
    }

    if (num_tokens == 1) {
        printf("setenv: too few arguments\n");
        return -1;
    }

    string value;
    if (num_tokens == 2)
        value = "\0";
    else
        value = tokens[num_tokens - 1];

    string key = tokens[1];

    int status = setenv(key.c_str(), value.c_str(), 69);
    if (status == -1)
        return -1;

    return 0;
}

int metash_unsetenv(vector<string> tokens) {
    size_t num_tokens = tokens.size();
    if (num_tokens >= 3) {
        printf("unsetenv: too many arguments\n");
        return -1;
    }

    if (num_tokens == 1) {
        printf("unsetenv: too few arguments\n");
        return -1;
    }

    string key = tokens[1];

    int status = unsetenv(key.c_str());
    if (status == -1)
        return -1;

    return 0;
}

int metash_getenv(vector<string> tokens) {
    size_t num_tokens = tokens.size();
    if (num_tokens >= 3) {
        printf("getenv: too many arguments\n");
        return -1;
    }

    if (num_tokens == 1) {
        printf("getenv: too few arguments\n");
        return -1;
    }

    string key = tokens[1];

    char* value = (char*)malloc(BUFSIZE * sizeof(char));
    value = getenv(key.c_str());

    if (value == NULL)
        value = (char*)"\0";

    printf("%s\n", value);

    return 0;
}
