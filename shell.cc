#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include <algorithm>

#include <readline/readline.h>
#include <readline/history.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins.h"
#include "tokenizer.h"
#include "utils.h"

using namespace std;

#define unused __attribute__((unused)) /* Silence compiler warnings about unused variables */
#define BUFSIZE 4096

#define RED "\x1b[31m"
#define GREEN "\x1b[92m"
#define GREENIT "\x1b[3;92m"
#define BLUE "\x1b[94m"
#define PURPLE "\x1b[35m"
#define CYAN "\x1b[96m"
#define YELLOW "\x1b[33m"
#define GRAY "\x1b[37m"
#define NORM "\x1B[0m"

/*
	__CWD: char []
		Contains the current working directory of the shell process
		Set on initializing the shell and then changes when `cd` is called
*/
extern char __CWD[BUFSIZE];

int shell_terminal = STDIN_FILENO;
pid_t shell_pgid = getpid();

// Check if a command is a builtin
int checkBuiltin(vector<string> tokens);

// Generate a prompt for the shell
const char *getShellPrompt();

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

	while ((line = readline(getShellPrompt())) != nullptr)
	{
		if (strlen(line) > 0)
			add_history(line);
		else
		{
			free(line);
			continue;
		}

		vector<string> tokens = tokenize(line);

		free(line);

		// Check if command is a builtin using the `checkBuiltin` call. If yes, execute it
		int isBuiltin = checkBuiltin(tokens);

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
		tokens.clear();
		fflush(stdin);
	}

	return 0;
}
