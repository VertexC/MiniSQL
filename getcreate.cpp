#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>

Package *_Interpreter::getCreateTable(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	CreateTPack *p = new CreateTPack();
	CreateTPack::Attri a;
	int state = 0;
	int flag = 0;
	char errMessage[200];
	vector<CreateTPack::Attri>::iterator it;//handle primary key

	p->type = Package::CREATET_PACK;
	while(!flag)
	{
		if (pos>=words.size())
		{
			sprintf(errMessage,"not finished at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
			error = error + errMessage;
			flag = 2;
			break;
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
				if (strlen(words[pos].str)>MAX_TABLE_LEN)
				{
					sprintf(errMessage,"table name too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				strcpy(p->table, words[pos].str);
				pos++;
				state = 1;
				break;
			case 1://(
				if (words[pos].type != Word::_LPAR)
				{
					sprintf(errMessage,"( expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 2;
				break;
			case 2://attribute name
				if (words[pos].type == Word::_PRIMARY)
				{
					state = 9;//key
					pos++;
					break;
				}
				else if( words[pos].type != Word::_LABLE)
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
				strcpy(a.name, words[pos].str);
				a.unique = false;
				pos++;
				state = 3;
				break;
			case 3://type
				if (words[pos].type == Word::_INT)
				{
					a.type = Package::INT_;
					a.size = sizeof(int);
					pos++;
					state = 7;//, or unique
				}
				else if (words[pos].type == Word::_FLOAT)
				{
					a.type = Package::FLOAT_;
					a.size = sizeof(double);
					pos++;
					state = 7;//, or unique
				}
				else if (words[pos].type == Word::_CHAR)
				{
					a.type = Package::CHAR_;
					pos++;
					state = 4;//(
				}
				else
				{
					sprintf(errMessage,"type expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				break;
			case 4://char(
				if (words[pos].type != Word::_LPAR)
				{
					sprintf(errMessage,"( expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 5;
				break;
			case 5://num
				if (words[pos].type != Word::_NUM)
				{
					sprintf(errMessage,"number expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				a.size = (int)words[pos].value+1;//+1
				pos++;
				state = 6;
				break;
			case 6://)
				if (words[pos].type != Word::_RPAR)
				{
					sprintf(errMessage,") expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 7;
				break;
			case 7://, or unique
				if (words[pos].type == Word::_UNIQUE)
				{
					a.unique = true;
					state = 8;
					pos++;
					break;
				}
				else if (words[pos].type != Word::_COMMA)
				{
					sprintf(errMessage,", expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				p->attri.push_back(a);
				pos++;
				state = 2;
				break;
			case 8://,
				if (words[pos].type != Word::_COMMA)
				{
					sprintf(errMessage,", expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				p->attri.push_back(a);
				pos++;
				state = 2;
				break;
			case 9://key
				if (words[pos].type != Word::_KEY)
				{
					sprintf(errMessage,"key expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 10;
				break;
			case 10://(
				if (words[pos].type != Word::_LPAR)
				{
					sprintf(errMessage,"( expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 11;
				break;
			case 11://attri
				if (words[pos].type != Word::_LABLE)
				{
					sprintf(errMessage,"attribute expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				for (it = p->attri.begin(); it != p->attri.end(); it++)
				{
					if (!strcmp(it->name, words[pos].str)) break;
				}
				if (it == p->attri.end())//if not exist in the define
				{
					sprintf(errMessage,"primary key not in the table at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				it->unique = true;
				if (strlen(words[pos].str)>MAX_CHAR_LEN)
				{
					sprintf(errMessage,"string too long at Line: %d, offsetL %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				strcpy(p->PK, words[pos].str);
				pos++;
				state = 12;
				break;
			case 12://)
				if (words[pos].type != Word::_RPAR)
				{
					sprintf(errMessage,") expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 13;
				break;
			case 13://)
				if (words[pos].type != Word::_RPAR)
				{
					sprintf(errMessage,") expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 14;
				break;
			case 14://;
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
	pos++;//the word after ;
	if (flag == 2)
	{
		delete p;
		return NULL;
	}
	return p;
}

Package *_Interpreter::getCreateIndex(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	char errMessage[200];
	CreateIPack *p = new CreateIPack();
	int state = 0;
	int flag = 0;

	p->type = Package::CREATEI_PACK;
	while(!flag)
	{
		if (pos>=words.size())
		{
			sprintf(errMessage,"not finished at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
			error = error + errMessage;
			flag = 2;
			break;
		}
		switch(state)
		{
			case -1://error
				if (words[pos].type != Word::_SEMI) pos++;
				else flag = 2;//error
				break;
			case 0:
				if (words[pos].type != Word::_LABLE)
				{
					sprintf(errMessage,"index name expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				strcpy(p->index, words[pos].str);
				pos++;
				state = 1;
				break;
			case 1://on
				if (words[pos].type != Word::_ON)
				{
					sprintf(errMessage,"on expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 2;
				break;
			case 2://table name
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
			case 4://attri
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
				strcpy(p->attri, words[pos].str);
				pos++;
				state = 5;
				break;
			case 5://)
				if (words[pos].type != Word::_RPAR)
				{
					sprintf(errMessage,") expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
					error = error + errMessage;
					state = -1;
					break;
				}
				pos++;
				state = 6;
				break;
			case 6://;
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

Package *_Interpreter::getCreate(const std::vector<Word> &words, unsigned int &pos, std::string &error)
{
	using namespace std;
	using namespace Interpreter;
	using namespace _Interpreter;
	char errMessage[200];

	if (pos<words.size() && words[pos].type == Word::_TABLE)
	{
		pos++;
		return getCreateTable(words, pos, error);
	}
	else if (pos<words.size() && words[pos].type == Word::_INDEX)
	{
		pos++;
		return getCreateIndex(words, pos, error);
	}
	sprintf(errMessage,"table or index expected at Line: %d, offset: %d\n", words[pos].line, words[pos].relativePos);
	error = error + errMessage;
	while(pos<words.size() && words[pos].type != Word::_SEMI) pos++;
	pos++;
	return NULL;
}