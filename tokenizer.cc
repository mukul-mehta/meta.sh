#include <cstring>

#include "tokenizer.h"

vector<string> tokenize(const char *line)
{
	vector<string> tokens;
	if (line == NULL)
	{
		return {};
	}

	static char token[MAXTOKEN];
	size_t n = 0;
	size_t len = strlen(line);

	const int MODE_NORMAL = 0,
			  MODE_SQUOTE = 1,
			  MODE_DQUOTE = 2;
	int mode = MODE_NORMAL;

	for (size_t i = 0; i < len; i++)
	{
		char c = line[i];
		if (mode == MODE_NORMAL)
		{
			if (c == '\'')
			{
				mode = MODE_SQUOTE;
			}
			else if (c == '"')
			{
				mode = MODE_DQUOTE;
			}
			else if (c == '\\')
			{
				if (i + 1 < len)
				{
					token[n++] = line[++i];
				}
			}
			else if (isspace(c))
			{
				if (n > 0)
				{
					token[n] = '\0';
					char *temp = (char *)malloc(n + 1);
					strncpy(temp, token, n + 1);
					string word = temp;
					tokens.push_back(word);
					n = 0;
				}
			}
			else
			{
				token[n++] = c;
			}
		}
		else if (mode == MODE_SQUOTE)
		{
			if (c == '\'')
			{
				mode = MODE_NORMAL;
			}
			else if (c == '\\')
			{
				if (i + 1 < len)
				{
					token[n++] = line[++i];
				}
			}
			else
			{
				token[n++] = c;
			}
		}
		else if (mode == MODE_DQUOTE)
		{
			if (c == '"')
			{
				mode = MODE_NORMAL;
			}
			else if (c == '\\')
			{
				if (i + 1 < len)
				{
					token[n++] = line[++i];
				}
			}
			else
			{
				token[n++] = c;
			}
		}
		if (n + 1 >= MAXTOKEN)
			abort();
	}

	if (n > 0)
	{
		string temp = token;
		tokens.push_back(temp);
		n = 0;
	}

	return tokens;
}

vector<vector<string>> parsePipeTokens(vector<string> tokens)
{
	vector<vector<string>> parsedTokens;
	// Create a temporary vector for a single token group. Fill it and add it to parsedTokens on pipe character
	vector<string> temp;
	size_t num_tokens = tokens.size();

	for (size_t i = 0; i < num_tokens; i++)
	{
		if (tokens[i] != "|")
		{
			temp.push_back(tokens[i]);
		}
		else
		{
			parsedTokens.push_back(temp);
			temp.clear();
		}
	}
	parsedTokens.push_back(temp);

	return parsedTokens;
}
