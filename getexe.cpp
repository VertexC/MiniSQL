#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>

Package *_Interpreter::getExe(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	char errMessage[200];
	int state = 0;
	int flag = 0;
	ExePack *p = new ExePack();

	p->type = Package::EXECFILE_PACK;
	while(!flag)
	{
		if (pos >= words.size())
		{
			sprintf(errMessage,"not finished at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
			error = error + errMessage;
			flag = 2;
		}
		switch(state)
		{
			case -1://error
				if (words[pos].type != Word::_SEMI) pos++;
				else flag = 2;//error
				break;
			case 0://filename
				if (words[pos].type != Word::_STRING)
				{
					sprintf(errMessage,"file expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				if (strlen(words[pos].str)>MAX_FILE_LEN)
				{
					sprintf(errMessage,"table name too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				strcpy(p->file, words[pos].str);
				pos++;
				state = 1;
				break;
			case 1://;
				if (words[pos].type != Word::_SEMI)
				{
					sprintf(errMessage,"; expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				flag = 1;
				break;
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