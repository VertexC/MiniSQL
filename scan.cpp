#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>
/*
*scan the input string, turn it into words, put error message in error
*return:false normal true error
*/
bool _Interpreter::scan(const std::string &s, std::vector<Word> &words, std::string &error)
{
	using namespace std;
	using namespace _Interpreter;
	int line = 1;
	unsigned int pos = 0;
	int relativePos = 1;
	Word word;
	bool errFlag = false;
	char errMessage[200];

	while(pos<s.length())
	{
		while(isspace(s[pos]))
		{
			if (s[pos] == '\n') 
			{
				line++;
				relativePos = 1;
			}
			else
			{
				relativePos++;
			}
			pos++;
		}//skip spaces
		if (pos >= s.length()) break;
		word.line = line;
		word.l = pos;
		word.relativePos = relativePos;
		word.str[0] = 0;
		word.value = 0;
		if (isalnum(s[pos]) || s[pos] == '_' || s[pos] == '\'') 
		{
			if (getWord(s, pos, line, relativePos, word, error))
			{
				while(!isspace(s[pos]))
				{
					pos++;
					relativePos++;
					//遇到第一个空格开始下一个单词
				}
				errFlag = true;
			}
			else
			{
				words.push_back(word);
			}
		}
		else if (s[pos] == ';')
		{
			word.r = pos;
			pos++;
			word.type = Word::_SEMI;
			words.push_back(word);
		}
		else if (s[pos] == ',')
		{
			word.r = pos;
			pos++;
			word.type = Word::_COMMA;
			words.push_back(word);
		}
		else if (s[pos] == '(')
		{
			word.r = pos;
			pos++;
			word.type = Word::_LPAR;
			words.push_back(word);
		}
		else if (s[pos] == ')')
		{
			word.r = pos;
			pos++;
			word.type  = Word::_RPAR;
			words.push_back(word);
		}
		else if (s[pos] == '=')
		{
			word.r = pos;
			pos++;
			word.type = Word::_EQU;
			words.push_back(word);
		}
		else if (s[pos] == '<' && s[pos+1] == '>')
		{
			word.r = ++pos;
			pos++;
			word.type = Word::_NOTEQU;
			words.push_back(word);
		}
		else if (s[pos] == '<' && s[pos+1] == '=')
		{
			word.r = ++pos;
			pos++;
			word.type = Word::_LESEQU;
			words.push_back(word);
		}
		else if (s[pos] == '>' && s[pos+1] == '=')
		{
			word.r = ++pos;
			pos++;
			word.type = Word::_GREEQU;
			words.push_back(word);
		}
		else if (s[pos] == '<')
		{
			word.r = pos;
			pos++;
			word.type = Word::_LES;
			words.push_back(word);
		}
		else if (s[pos] == '>')
		{
			word.r = pos;
			pos++;
			word.type = Word::_GRE;
			words.push_back(word);
		}
		
		else if (s[pos] == '*')
		{
			word.r = pos;
			pos++;
			word.type = Word::_STAR;
			std::strcpy(word.str,"*");
			words.push_back(word);
		}
		else
		{
			sprintf(errMessage, "Unknown character at Line: %d, offset: %d\n", line, relativePos);
			error = error + errMessage;
			while(!isspace(s[pos]))
			{
				pos++;
				relativePos++;
				//遇到第一个空格开始下一个单词
			}
			errFlag = true;
		}
	}
	return errFlag;
}

