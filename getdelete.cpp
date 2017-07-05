#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>

Package * _Interpreter::getDelete(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	char errMessage[200];
	int state = 0;
	int flag = 0;
	DeletePack *p = new DeletePack();
	DeletePack::Condition c;

	p->type = Package::DELETE_PACK;
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
			case 0://from
				if (words[pos].type != Word::_FROM)
				{
					sprintf(errMessage,"from expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
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
			case 2://; or where
				if (words[pos].type == Word::_WHERE)
				{
					state = 3;
					pos++;
					break;
				}
				else if (words[pos].type == Word::_SEMI)
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
				pos++;
				break;
			case 3://attri
				if (words[pos].type != Word::_LABLE)
				{
					sprintf(errMessage,"attribute expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				if (strlen(words[pos].str)>MAX_ATTRI_LEN)
				{
					sprintf(errMessage,"attribute name too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				strcpy(c.attri, words[pos].str);
				pos++;
				state = 4;
				break;
			case 4://op
				if (words[pos].type == Word::_LES)
					c.op = Package::LES_;
				else if (words[pos].type == Word::_EQU)			
					c.op = Package::EQU_;			
				else if (words[pos].type == Word::_GRE)			
					c.op = Package::GRE_;			
				else if (words[pos].type == Word::_LESEQU)				
					c.op = Package::LESEQU_;				
				else if (words[pos].type == Word::_GREEQU)				
					c.op = Package::GREEQU_;				
				else if (words[pos].type == Word::_NOTEQU)				
					c.op = Package::NOTEQU_;
				else
				{
					sprintf(errMessage,"compare expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 5;
				break;
			case 5://value
				if (words[pos].type == Word::_NUM)
				{
					c.type = Package::INT_;
					c.value.i = (int)words[pos].value;
				}
				else if (words[pos].type == Word::_DOUBLE)
				{
					c.type = Package::FLOAT_;
					c.value.f = words[pos].value;
				}
				else if (words[pos].type == Word::_STRING)
				{
					c.type = Package::CHAR_;
					if (strlen(words[pos].str)>MAX_CHAR_LEN)
					{
						sprintf(errMessage,"string too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
						error = error + errMessage;
						state = -1;
						break;
					}
					strcpy(c.value.c, words[pos].str);
				}
				else
				{
					sprintf(errMessage,"value expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 6;
				break;
			case 6://and or ;
				if (words[pos].type == Word::_AND)
				{
					p->condition.push_back(c);
					pos++;
					state = 3;
					break;
				}
				else if(words[pos].type == Word::_SEMI)
				{
					p->condition.push_back(c);
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
	if (flag ==2)
	{
		delete p;
		return NULL;
	}
	return p;
}