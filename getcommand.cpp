#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>

Package *_Interpreter::getCommand(const std::vector<Word> &words, std::string &error)
{
	using namespace std;
	using namespace _Interpreter;
	char errMessage[200];
	unsigned int pos = 0;
	if (words[0].type == Word::_CREATE)
	{
		return getCreate(words, ++pos, error);
	}
	else if (words[0].type == Word::_DROP)
	{
		return getDrop(words, ++pos, error);
	}
	else if(words[0].type == Word::_SELECT)
	{
		return getSelect(words, ++pos, error);
	}
	else if(words[0].type == Word::_INSERT)
	{
		return getInsert(words, ++pos, error);
	}
	else if (words[0].type == Word::_DELETE)
	{
		return getDelete(words, ++pos, error);
	}
	else if (words[0].type == Word::_QUIT)
	{
		return getQuit(words, ++pos, error);
	}
	else if (words[0].type == Word::_EXECFILE)
	{
		return getExe(words, ++pos, error);
	}
	sprintf(errMessage,"Unknown command at Line: %d, offset: %d\n", words[0].line, words[0].relativePos);//l和r似乎是没有用的
	error = error + errMessage;
	while(pos<words.size() && words[pos].type != Word::_SEMI) pos++;
	pos++;
	return NULL;
}
