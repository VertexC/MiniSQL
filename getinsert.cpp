#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>

Package *_Interpreter::getInsert(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	char errMessage[200];
	int state = 0;
	int flag = 0;
	InsertPack *p = new InsertPack();
	InsertPack::Value v;

	p->type = Package::INSERT_PACK;
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
			case 0://into
				if (words[pos].type != Word::Word::_INTO)
				{
					sprintf(errMessage,"into expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 1;
				break;
			case 1://table name
				if (words[pos].type != Word::_LABLE)
				{
					sprintf(errMessage,"table name expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				if (strlen(words[pos].str)>MAX_TABLE_LEN)
				{
					sprintf(errMessage,"table name too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				strcpy(p->table, words[pos].str);
				pos++;
				state = 2;
				break;
			case 2://values
				if (words[pos].type != Word::_VALUES)
				{
					sprintf(errMessage,"values expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 3;
				break;
			case 3://(
				if (words[pos].type != Word::_LPAR)
				{
					sprintf(errMessage,"( expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 4;
				break;
			case 4://value;
				if (words[pos].type == Word::_NUM)
				{
					v.type = Package::INT_;
					v.size = sizeof(int);
					v.value.i = (int)words[pos].value;
				}
				else if (words[pos].type == Word::_DOUBLE)
				{
					v.type = Package::FLOAT_;
					v.size = sizeof(double);
					v.value.f = words[pos].value;
				}
				else if (words[pos].type == Word::_STRING)
				{
					v.type = Package::CHAR_;
					v.size = strlen(words[pos].str)+1;
					if (strlen(words[pos].str)>MAX_CHAR_LEN)
					{
						sprintf(errMessage,"string too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
						error = error + errMessage;
						state = -1;
						break;
					}
					strcpy(v.value.c, words[pos].str);
				}
				else
				{
					sprintf(errMessage,"value expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 5;
				break;
			case 5://, or )
				if (words[pos].type == Word::_COMMA)
				{
					p->value.push_back(v);
					pos++;
					state = 4;
				}
				else if (words[pos].type == Word::_RPAR)
				{
					p->value.push_back(v);
					pos++;
					state = 6;
				}
				else
				{
					sprintf(errMessage,") expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				break;
			case 6://;
				if (words[pos].type == Word::_SEMI)
				{
					flag = 1;
					break;
				}
				else
				{
					sprintf(errMessage,"; expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
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