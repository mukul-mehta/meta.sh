#include <string.h>

#include <iostream>
#include <algorithm>
#include <fstream>

#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include "tokenizer.h"

using namespace std;

#define unused __attribute__((unused)) /* Silence compiler warnings about unused variables */
#define BUFSIZE 4096
#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR

#define RED "\x1b[31m"
#define GREEN "\x1b[92m"
#define GREENIT "\x1b[3;92m"
#define BLUE "\x1b[94m"
#define PURPLE "\x1b[35m"
#define CYAN "\x1b[96m"
#define YELLOW "\x1b[33m"
#define GRAY "\x1b[37m"
#define NORM "\x1B[0m"

#define SHELL "meta.sh"
#define VERSION "0.1"
#define AUTHORS "Mukul Mehta | Rashil Gandhi"

/*
	struct builtinFunction
	Handle shell builtins along with their help string
	------------------
	Members:
		builtin_fp: int (*)(vector<string> ) -> Function pointer to the builtin handler
		command: string -> The command that executes the builtin, by calling the correct handler
		help: string -> Doc about the command, displayed when the builtin `help` is called
	------------------
*/
struct builtinFunction
{
	int (*builtin_fp)(vector<string> tokens);
	string command;
	string help;
};

// FUNCTION PROTOTYPES
/*
	metash functions -> Same prototype: int (vector<string>)
	Used to execute builtins and help withexternal commands
	Takes input a vector of string. After taking input from user, the raw input is
	tokenized into a vector of string and is passed to this class of functions
*/

/*
	int metash_exit(vector<string> tokens)
	------------------
	Exits the shell. Arguments unused
*/
int metash_exit(unused vector<string> tokens);

/*
	int metash_help(vector<string> tokens)
	------------------
	Display help menu with information about builtins. Arguments unused
*/
int metash_help(vector<string> tokens);

/*
	int metash_pwd(vector<string> tokens)
	------------------
	Prints the current working directory of the shell process using the `chdir()` call defined in <unistd.h>
	If additional parameters are passed, prints an error and returns -1
*/
int metash_pwd(vector<string> tokens);

/*
	int metash_cd(vector<string> tokens)
	------------------
	On successfully changing directory, the global variable `__CWD` is updated and 0 is returned

	Parameters:
	------------------
	tokens: vector<string>
		A vector of tokens after tokenizing user's input. If the length is 2, the
		second token is taken to be the directory the user wants to change to
		If there is just one token, cd changes to the user's home directory, obtained using `getpwuid` from <pwd.h>
		If >= 3 arguments are passed, prints an error and returns -1
*/
int metash_cd(vector<string> tokens);

/*
	int metash_fetch(vector<string> tokens)
	------------------
	Prints out system information obtained from different headers
	Username, Hostname and OS Name: From the getUsername, getHostname and getOSName functions
	Kernel and Platform: Fills the `utsname` struct using the `uname` call from <sys/utsname.h>
	Uptime and Memory: Fills the `sysinfo` struct using the `sysinfo` call from <sys/sysinfo.h>
*/
int metash_fetch(unused vector<string> tokens);

/*
	int metash_execute(vector<string> tokens)
	------------------
	Given a vector of strings, handle I/O file redirection and execute using the `execvp` call

	Parameters:
	------------------
	tokens: vector<string>
		The first entry contains the program name and other entries are command line arguments or
		redirection arguments. Before calling the appropriate executable with `execvp`, check for
		input and output redirection. If found, get the file names and open file descriptors for them.
		To redirect, use the `dup2` system call and duplicate the file descriptors with the new FDs
		set to STDIN_FILENO or STDOUT_FILENO depending on what is being redirected
		If the call to `execvp` fails, print an error and return -1
*/
int metash_execute(vector<string> tokens);

// Helper Functions
// Fetch Username, Hostname and OS Name from appropriate headers
string getUsername();
string getHostname();
string getOSName();

// Check if a command is a builtin
int checkBuiltin(vector<string> tokens);

// Generate a prompt for the shell
const char *getShellPrompt();

// GLOBAL VARIABLES
/*
	__CWD: char []
		Contains the current working directory of the shell process
		Set on initializing the shell and then changes when `cd` is called
*/
char __CWD[BUFSIZE];

/*
	builtins: vector<builtinFunctions>
		Vector of all builtin functions implemented by the shell
		When a command is to be executed, compares with this list to check if builtin
*/
vector<builtinFunction> builtins = {
    {metash_cd, "cd", "Changes working directory to the one specified"},
    {metash_pwd, "pwd", "Shows current working directory "},
    {metash_help, "help", "Shows this help text"},
    {metash_exit, "exit", "Cleanly exits the shell"},
    {metash_fetch, "fetch", "Show system information"},
};

int shell_terminal = STDIN_FILENO;
pid_t shell_pgid = getpid();

// FUNCTION DEFINITIONS

int metash_exit(unused vector<string> tokens)
{
	exit(EXIT_SUCCESS);
}

int metash_help(vector<string> tokens)
{
	printf("%s++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s\n", PURPLE, NORM);
	printf("%s++++++%s /\\        %sHi there. There are \033[4mfive%s %sbuiltin commands%s     /\\ %s++++++%s\n", PURPLE, NORM, CYAN, NORM, CYAN, NORM, PURPLE, NORM);
	printf("%s++++++%s cd%s:    Changes working directory to the one specified      %s++++++%s\n", PURPLE, YELLOW, NORM, PURPLE, NORM);
	printf("%s++++++%s pwd%s:   Shows current working directory                     %s++++++%s\n", PURPLE, YELLOW, NORM, PURPLE, NORM);
	printf("%s++++++%s help%s:  Shows this help text                                %s++++++%s\n", PURPLE, YELLOW, NORM, PURPLE, NORM);
	printf("%s++++++%s exit%s:  Cleanly exits the shell                             %s++++++%s\n", PURPLE, YELLOW, NORM, PURPLE, NORM);
	printf("%s++++++%s fetch%s: Show system information                             %s++++++%s\n", PURPLE, YELLOW, NORM, PURPLE, NORM);
	printf("%s++++++%s Anything else is considered as an executable, and should   %s++++++%s\n", PURPLE, BLUE, PURPLE, NORM);
	printf("%s++++++%s be present in your PATH. \033[1;3;34mEnjoy!%s                            %s++++++%s\n", PURPLE, BLUE, NORM, PURPLE, NORM);
	printf("%s++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s\n", PURPLE, NORM);

	return 0;
}

int metash_pwd(vector<string> tokens)
{
	size_t num_tokens = tokens.size();
	if (num_tokens >= 2)
	{
		printf("pwd: too many arguments\n");
		return -1;
	}

	char *current_directory = (char *)malloc(BUFSIZE * sizeof(char));
	string pwd = getcwd(current_directory, BUFSIZE);
	if (!pwd.empty())
	{
		cout << pwd << endl;
		return 0;
	}
	perror("Error in fetching current working directory");
	return -1;
}

int metash_cd(vector<string> tokens)
{
	size_t num_tokens = tokens.size();
	if (num_tokens >= 3)
	{
		cout << "cd : too many arguments" << endl;
		return -1;
	}

	string new_directory_rel;
	char *new_directory = (char *)malloc(BUFSIZE * sizeof(char));
	// If no argument is passed, assume cd to home directory of user. Fetch home directory path
	if (num_tokens == 1)
	{
		uid_t uid = getuid();
		struct passwd *pw = getpwuid(uid);
		new_directory_rel = pw->pw_dir; // Stores the home direcoty path of the user executing the program
	}
	else
	{
		new_directory_rel = tokens[1];
	}
	// In case of links, or if a relative path is specified, generate the real path
	char *temp = realpath(new_directory_rel.c_str(), new_directory);
	if (!temp)
		perror("Couldn't get realpath");

	int ret = chdir(new_directory);
	if (ret < 0)
	{
		printf("Error in changing current directory. pwd not changed");
		return -1;
	}
	strcpy(__CWD, new_directory);
	return 0;
}

int metash_fetch(unused vector<string> tokens)
{
	// Fetch username, hostname and OS Name
	unused string username = getUsername();
	unused string hostname = getHostname();
	unused string OSname = getOSName();

	string kernel_name,
	    kernel_version, machine;

	long uptime = -1;
	long total_memory = -1;
	long free_memory = -1;
	long consumed_memory = -1;

	struct utsname uts;
	// The `uname` call populates the uts struct with system information. Also implemented as the `uname` executable
	if (uname(&uts) < 0)
	{
		perror("uname error");
	}
	else
	{
		kernel_name = uts.sysname;
		kernel_version = uts.release;
		machine = uts.machine;
	}

	struct sysinfo info;
	// The `sysinfo` call populates the `sysinfo` struct. Fills with uptime and memory usage information
	if (sysinfo(&info) < 0)
		perror("sysinfo() error");
	else
	{
		total_memory = info.totalram;
		free_memory = info.freeram;
		consumed_memory = total_memory - free_memory;
		uptime = info.uptime;
	}

	char *user_host_string = (char *)malloc(BUFSIZE * sizeof(char));
	sprintf(user_host_string, "%s%s@%s%s\n", BLUE, username.c_str(), hostname.c_str(), NORM);
	size_t len = username.size() + hostname.size();

	printf(user_host_string);
	for (size_t i = 0; i <= len; i++)
		printf("-");
	printf("\n");

	printf("%sOS%s:       %s\n", BLUE, NORM, OSname.c_str());
	printf("%sKernel%s:   %s %s\n", BLUE, NORM, kernel_name.c_str(), kernel_version.c_str());
	printf("%sPlatform%s: %s\n", BLUE, NORM, machine.c_str());
	printf("%sMemory%s:   %s / %s\n", BLUE, NORM, parse_memory(consumed_memory), parse_memory(total_memory));
	printf("%sUptime%s:   %s\n", BLUE, NORM, parse_time(uptime));
	printf("%sShell%s:    %s %s\n", BLUE, NORM, SHELL, VERSION);
	printf("%sAuthors%s:  %s\n", BLUE, NORM, AUTHORS);

	return 0;
}

int metash_execute(vector<string> tokens)
{
	/*
		Check if there is a < token in the token vector. If found, the next token of < will be the filename
		Remove this token and the filename since the execvp call needs only the arguments passed
		Open the file and use the `dup2` system call to set appropriate file descriptor entry
	*/

	vector<string>::iterator inpIter = find(tokens.begin(), tokens.end(), "<");
	if (inpIter != tokens.end())
	{
		int index = inpIter - tokens.begin();
		int inputFD = open(tokens[index + 1].c_str(), O_RDONLY);
		int status = dup2(inputFD, READ_FLAGS);
		if (status == -1)
			perror("dup2() failed");

		tokens.erase(inpIter, inpIter + 2);
	}

	// Same as above, check for existence of > token and use the `dup2` call with the output file
	vector<string>::iterator outIter = find(tokens.begin(), tokens.end(), ">");
	if (outIter != tokens.end())
	{
		int index = outIter - tokens.begin();
		int outputFD = open(tokens[index + 1].c_str(), WRITE_FLAGS, 0644);
		int status = dup2(outputFD, STDOUT_FILENO);
		if (status == -1)
			perror("dup2() failed");

		tokens.erase(outIter, outIter + 2);
	}

	size_t num_tokens = tokens.size();
	// `execvp` requires a char array with the last element set to NULL
	char *args[num_tokens + 1];
	for (size_t i = 0; i < tokens.size(); i++)
		args[i] = (char *)(tokens[i].c_str());

	args[num_tokens] = NULL;

	int ret = execvp(tokens[0].c_str(), args);
	if (ret == -1)
	{
		printf("execvp() failed: Command not found: %s\n", tokens[0].c_str());
	}
	exit(EXIT_FAILURE);
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

int checkBuiltin(vector<string> tokens)
{
	// Iterate over builtins and check if the command is same as the first token. If found, return index else -1
	string command = tokens[0];
	size_t n = builtins.size();
	for (size_t i = 0; i < n; i++)
	{
		if (builtins[i].command == command)
			return i;
	}
	return -1;
}

const char *getShellPrompt()
{
	string username = getUsername();
	string hostname = getHostname();
	char timeBuffer[12];

	auto t = time(nullptr);
	struct tm *ltime = localtime(&t);
	strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", ltime);

	char *prompt = (char *)malloc(BUFSIZE * sizeof(char));

	// Check width of terminal
	// Needed because some part of the prompt is on the right end and need to fill with correct num spaces in between
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	int width = size.ws_col;
	int numSpaces = width - (strlen(__CWD) + 2) - (hostname.size()) - (strlen(timeBuffer));

	string spaces(numSpaces, ' ');
	sprintf(prompt, ": %s%s%s%s%s%s%s [%s%s@%s%s] \n%s@%s ", GREEN, __CWD, NORM, spaces.c_str(), GRAY, timeBuffer, NORM, CYAN, username.c_str(), hostname.c_str(), NORM, RED, NORM);
	sprintf(prompt, "%s%s%s: %s%s%s%s%s%s%s\n%s%s%s %s@%s ", YELLOW, hostname.c_str(), NORM, GREENIT, __CWD, NORM, spaces.c_str(), GRAY, timeBuffer, NORM, CYAN, username.c_str(), NORM, RED, NORM);
	return prompt;
}

int main(int argc, char **argv)
{
	metash_help(vector<string>{});
	getcwd(__CWD, BUFSIZE);

	char *line;
	size_t getline_size = 0;

	printf("%s", getShellPrompt());
	while (getline(&line, &getline_size, stdin))
	{
		if (feof(stdin))
			exit(EXIT_SUCCESS);

		vector<string> tokens = tokenize(line);
		if (tokens.empty())
		{
			printf("%s", getShellPrompt());
			continue;
		}

		// Check if command is a builtin using the `checkBuiltin` call. If yes, execute it
		unused int isBuiltin = checkBuiltin(tokens);

		if (isBuiltin >= 0)
		{
			builtinFunction builtin = builtins[isBuiltin];
			builtin.builtin_fp(tokens);
		}
		else
		{
			// If the last token of the input is &, the command is to be run in background
			// Set the bool isBackground and remove the token because execvp does not need it
			unused bool isBackground = false;
			if (tokens[tokens.size() - 1] == "&")
			{
				isBackground = true;
				tokens.pop_back();
			}

			// If there is a pipe character, set isPipe to true. Piped inputs are handled differently
			bool isPipe = false;
			vector<string>::iterator it = find(tokens.begin(), tokens.end(), "|");
			if (it != tokens.end())
			{
				isPipe = true;
			}

			if (!isPipe)
			{
				/*
					Execute all commands that don't have any pipe in them

					To execute a command, use the usual fork/exec method. A child process
					is spawned and then we pass the tokens to `metash_execute` which runs `execvp`
					in the child, replacing it with the desired program

					In the parent, if the process was not a background process, wait for the
					child to finish executing before issuing a prompt again
				*/
				pid_t pid = fork();
				int status;

				if (pid == 0)
				{
					if (isBackground)
						setpgid(0, 0);
					metash_execute(tokens);
				}
				else if (pid > 0)
				{
					if (!isBackground)
					{
						if (setpgid(pid, pid) == -1)
							perror("setpgid() failed");

						if (tcsetpgrp(shell_terminal, pid) == 0)
						{
							if ((waitpid(pid, &status, WUNTRACED)) < 0)
							{
								perror("wait() failed");
								exit(EXIT_FAILURE);
							}

							signal(SIGTTOU, SIG_IGN);
							if (tcsetpgrp(shell_terminal, shell_pgid) != 0)
								perror("tcsetpgrp() failed");
							signal(SIGTTOU, SIG_DFL);
						}
						else
						{
							perror("tcsetpgrp() failed");
						}
					}
				}
				else
				{
					perror("fork() failed");
				}
			}
			else
			{
				/*
					Execute all commands that have pipes in them

					Firstly we split the tokens into token groups, with each group representing
					one command in the pipe. Then we iterate over each of these commands
					We use the `pipe` system call to create a pipe. Then the correct file
					descriptors are set. For a command of form `a | b | c`, the output of a is
					set as the input of b, the output of b is set as input of c and so on

					After setting descriptors, the usual fork/exec method is called and the function
					`metash_execute` is called. The parent closes file descriptors as we progress to
					the next command in the pipe call

					Once all pipes have been set, we wait for all children to finish execution
				*/
				vector<vector<string>> parsedTokens = parsePipeTokens(tokens);

				size_t num_commands = parsedTokens.size();
				int pipeFD[2];
				int tempFD[2];

				for (size_t i = 0; i < num_commands; i++)
				{
					if (i != num_commands - 1)
						pipe(pipeFD);

					pid_t pid = fork();

					if (pid == 0)
					{
						if (i != 0)
						{
							int dupStatus = dup2(tempFD[0], STDIN_FILENO);
							if (dupStatus == -1)
								perror("dup2() failed");

							close(tempFD[0]);
							close(tempFD[1]);
						}

						if (i != num_commands - 1)
						{
							int dupStatus = dup2(pipeFD[1], STDOUT_FILENO);
							if (dupStatus == -1)
								perror("dup2() failed");

							close(pipeFD[0]);
							close(pipeFD[1]);
						}

						metash_execute(parsedTokens[i]);
					}
					else if (pid > 0)
					{

						if (i != 0)
						{
							close(tempFD[0]);
							close(tempFD[1]);
						}

						if (i != num_commands - 1)
						{
							tempFD[0] = pipeFD[0];
							tempFD[1] = pipeFD[1];
						}
					}
					else
					{
						perror("fork() failed");
					}
				}
				if (!isBackground)
				{
					int ret;
					do
					{
						ret = wait(NULL);
					} while (ret > 0);
				}
			}
		}
		cout << endl;
		printf("%s", getShellPrompt());
		tokens.clear();
		getline_size = 0;
	}

	return 0;
}
