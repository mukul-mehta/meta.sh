#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <vector>
#include <string>

#define MAXTOKEN 1024

/*
	vector<string> tokenize(const char *line)
	------------------
	Tokenize the input line into a vector of strings and return

	Parameters:
	------------------
	line: const char *
		The input as received by the `readline` function call made in the main shell loop

	Returns:
	------------------
	tokens: vector<string>
		A list of tokens generated from the input line

	The tokenizer escapes quotes, spaces and the backslash character

*/
std::vector<std::string> tokenize(char* line);

/*
	vector<vector<string>> parsePipeTokens(vector<string> tokens)
	------------------
	In case the input contains pipes, split around the pipe character into individual
	token groups. For example the input [cat, a.txt, |, grep, "some text", |, wc] is split as
	[[cat, a.txt], [grep, "some text"], [wc]]
*/

std::vector<std::vector<std::string>> parsePipeTokens(std::vector<std::string> tokens);
#endif // TOKENIZER_H_