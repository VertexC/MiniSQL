#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>

Package *_Interpreter::getQuit(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	int flag = 0;
	int state = 0;
	char errMessage[200];
	Package *p = new Package();
	p->type = Package::QUIT_PACK;
	while(!flag)
	{
		if (pos >= words.size())
		{
			sprintf(errMessage,"not finished at Line: %d, offset: %d\n", words[0].line, words[0].relativePos);//l和r似乎是没有用的
			error = error + errMessage;
			flag = 2;
		}
		if (state == 0)
		{
			if (words[1].type == Word::_SEMI)
			{
				flag = 1;
			}
			else
			{
				state = -1;
			}
		}
		else if (state == -1)
		{
			if (words[pos].type != Word::_SEMI) pos++;
		}
	}
	pos++;
	if (flag == 2)
	{
		delete p;
		return NULL;
	}
	return p;
}