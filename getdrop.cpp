#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>

Package *_Interpreter::getDropTable(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	char errMessage[200];
	DropTPack *p = new DropTPack();
	int state = 0;
	int flag = 0;

	p->type = Package::DROPT_PACK;
	while (!flag)
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
			case 0://table name
				if (words[pos].type != Word::_LABLE)
				{
					sprintf(errMessage,"table name expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				state = 1;
				if (strlen(words[pos].str)>MAX_TABLE_LEN)
				{
					sprintf(errMessage,"table name too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				strcpy(p->table, words[pos].str);
				pos++;
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

Package *_Interpreter::getDropIndex(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	char errMessage[200];
	DropIPack *p = new DropIPack();
	int state = 0;
	int flag = 0;

	p->type = Package::DROPI_PACK;
	while (!flag)
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
			case 0://index name
				if (words[pos].type != Word::_LABLE)
				{
					sprintf(errMessage,"index name expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				state = 1;
				if (strlen(words[pos].str)>MAX_INDEX_LEN)
				{
					sprintf(errMessage,"table name too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				strcpy(p->index, words[pos].str);
				pos++;
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

Package *_Interpreter::getDrop(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	char errMessage[200];

	if (pos<words.size() && words[pos].type == Word::_TABLE)
	{
		pos++;
		return getDropTable(words, pos, error);
	}
	else if (words[pos].type == Word::_INDEX)
	{
		pos++;
		return getDropIndex(words, pos, error);
	}
	sprintf(errMessage,"table or index expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
	error = error + errMessage;
	while(pos<words.size() && words[pos].type != Word::_SEMI) pos++;
	pos++;
	return NULL;
}