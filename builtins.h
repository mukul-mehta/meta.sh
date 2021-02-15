#ifndef BUILTINS_H_
#define BUILTINS_H_

#include <string>
#include <vector>

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
	int (*builtin_fp)(std::vector<std::string> tokens);
	std::string command;
	std::string help;
};

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
int metash_exit(unused std::vector<std::string> tokens);

/*
	int metash_help(vector<string> tokens)
	------------------
	Display help menu with information about builtins. Arguments unused
*/
int metash_help(std::vector<std::string> tokens);

/*
	int metash_pwd(vector<string> tokens)
	------------------
	Prints the current working directory of the shell process using the `chdir()` call defined in <unistd.h>
	If additional parameters are passed, prints an error and returns -1
*/
int metash_pwd(std::vector<std::string> tokens);

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
int metash_cd(std::vector<std::string> tokens);

/*
	int metash_fetch(vector<string> tokens)
	------------------
	Prints out system information obtained from different headers
	Username, Hostname and OS Name: From the getUsername, getHostname and getOSName functions
	Kernel and Platform: Fills the `utsname` struct using the `uname` call from <sys/utsname.h>
	Uptime and Memory: Fills the `sysinfo` struct using the `sysinfo` call from <sys/sysinfo.h>
*/
int metash_fetch(unused std::vector<std::string> tokens);

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
int metash_execute(std::vector<std::string> tokens);

/*
	int metash_history(vector<string> tokens)
	------------------
	Print a numbered list of all commands executed on the shell, not limited to the present session

*/
int metash_history(unused std::vector<std::string> tokens);

#endif // BUILTINS_H_
