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
