#include "interpreter_internal.h"
#include <cstdio>
#include <cstring>

//seperators between words or end of string
bool _Interpreter::isSeperator(char c)
{
	using namespace std;
	return c == 0 || isspace(c) || c == '(' || c == ')' || c == ';' 
			|| c == '<' || c == '>' || c == '=' || c == ',' || c== '\'';
}

/*
*get a word from s start at pos, and store the information in word, error message in error
*line and relativePos is the position in the input, for beautiful error message
*return:0 normal 1 error
*/
int _Interpreter::getWord(const std::string &s, unsigned int &pos, int &line, int &relativePos, Word &word, std::string &error)
{
	using namespace std;
	using namespace _Interpreter;
	int state = 0;
	int flag = 0;//1 normal 2 error
	double sum = 0;//for num
	double exp = 1;//for float
	string temps;//for lable
	char errMessage[200];//temp string

	word.l = pos;
	word.line = line;
	word.relativePos = relativePos;
	while(!flag)
	{
		switch(state)
		{
			case 0: 
				temps.clear();
				//key words first
				if (pos+2<s.length() && s[pos] == 'a' && s[pos+1] == 'n' && s[pos+2] == 'd' && isSeperator(s[pos+3]))
				{
					word.type = Word::_AND;
					pos+=3;
					relativePos += 3;
					flag = 1;
				}
				else if (pos+3<s.length() && s[pos] == 'c' && s[pos+1] == 'h' && s[pos+2] == 'a' && s[pos+3] == 'r' 
						&& isSeperator(s[pos+4]))
				{
					word.type = Word::_CHAR;
					pos+=4;
					relativePos += 4;
					flag = 1;
				}
				else if (pos+5<s.length() && s[pos] == 'c' && s[pos+1] == 'r' && s[pos+2] == 'e' && s[pos+3] == 'a' 
						&& s[pos+4] == 't' && s[pos+5] == 'e' && isSeperator(s[pos+6]))
				{
					word.type = Word::_CREATE;
					pos+=6;
					relativePos += 6;
					flag = 1;
				}
				else if (pos+3<s.length() && s[pos] == 'd' && s[pos+1] == 'r' && s[pos+2] == 'o' && s[pos+3] == 'p' 
						&&isSeperator(s[pos+4]))
				{
					word.type = Word::_DROP;
					pos+=4;
					relativePos += 4;
					flag = 1;
				}
				else if (pos+7<s.length() && s[pos] == 'e' && s[pos+1] == 'x' && s[pos+2] == 'e' && s[pos+3] == 'c' 
						&& s[pos+4] == 'f' && s[pos+5] == 'i' && s[pos+6] == 'l' && s[pos+7] == 'e' && isSeperator(s[pos+8]))
				{
					word.type = Word::_EXECFILE;
					pos+=8;
					relativePos += 8;
					flag = 1;
				}
				else if (pos+4<s.length() && s[pos] == 'f' && s[pos+1] == 'l' && s[pos+2] == 'o' && s[pos+3] == 'a' 
						&& s[pos+4] == 't' && isSeperator(s[pos+5]))
				{
					word.type = Word::_FLOAT;
					pos+=5;
					relativePos += 5;
					flag = 1;
				}
				else if (pos+3<s.length() && s[pos] == 'f' && s[pos+1] == 'r' && s[pos+2] == 'o' && s[pos+3] == 'm' 
						&&isSeperator(s[pos+4]))
				{
					word.type = Word::_FROM;
					pos+=4;
					relativePos += 4;
					flag = 1;
				}
				else if (pos+4<s.length() && s[pos] == 'i' && s[pos+1] == 'n' && s[pos+2] == 'd' && s[pos+3] == 'e' 
						&& s[pos+4] == 'x' && isSeperator(s[pos+5]))
				{
					word.type = Word::_INDEX;
					pos+=5;
					relativePos += 5;
					flag = 1;
				}
				else if (pos+2<s.length() && s[pos] == 'i' && s[pos+1] == 'n' && s[pos+2] == 't' && isSeperator(s[pos+3]))
				{
					word.type = Word::_INT;
					pos+=3;
					relativePos += 3;
					flag = 1;
				}
				else if (pos+2<s.length() && s[pos] == 'k' && s[pos+1] == 'e' && s[pos+2] == 'y' && isSeperator(s[pos+3]))
				{
					word.type = Word::_KEY;
					pos+=3;
					relativePos += 3;
					flag = 1;
				}
				else if (pos+1<s.length() && s[pos] == 'o' && s[pos+1] == 'n' && isSeperator(s[pos+2]))
				{
					word.type = Word::_ON;
					pos+=2;
					relativePos += 2;
					flag = 1;
				}
				else if (pos+6<s.length() && s[pos] == 'p' && s[pos+1] == 'r' && s[pos+2] == 'i' && s[pos+3] == 'm' 
						&& s[pos+4] == 'a' && s[pos+5] == 'r' && s[pos+6] == 'y' && isSeperator(s[pos+7]))
				{
					word.type = Word::_PRIMARY;
					pos+=7;
					relativePos += 7;
					flag = 1;
				}
				else if (pos+3<s.length() && s[pos] == 'q' && s[pos+1] == 'u' && s[pos+2] == 'i' && s[pos+3] == 't' 
						&&isSeperator(s[pos+4]))
				{
					word.type = Word::_QUIT;
					pos+=4;
					relativePos += 4;
					flag = 1;
				}
				else if (pos+5<s.length() && s[pos] == 's' && s[pos+1] == 'e' && s[pos+2] == 'l' && s[pos+3] == 'e' 
						&& s[pos+4] == 'c' && s[pos+5] == 't' && isSeperator(s[pos+6]))
				{
					word.type = Word::_SELECT;
					pos+=6;
					relativePos += 6;
					flag = 1;
				}
				else if (pos+4<s.length() && s[pos] == 't' && s[pos+1] == 'a' && s[pos+2] == 'b' && s[pos+3] == 'l' 
						&& s[pos+4] == 'e' && isSeperator(s[pos+5]))
				{
					word.type = Word::_TABLE;
					pos+=5;
					relativePos += 5;
					flag = 1;
				}
				else if (pos+5<s.length() && s[pos] == 'u' && s[pos+1] == 'n' && s[pos+2] == 'i' && s[pos+3] == 'q' 
						&& s[pos+4] == 'u' && s[pos+5] == 'e' && isSeperator(s[pos+6]))
				{
					word.type = Word::_UNIQUE;
					pos+=6;
					relativePos += 6;
					flag = 1;
				}
				else if (pos+4<s.length() && s[pos] == 'w' && s[pos+1] == 'h' && s[pos+2] == 'e' && s[pos+3] == 'r' 
						&& s[pos+4] == 'e' && isSeperator(s[pos+5]))
				{
					word.type = Word::_WHERE;
					pos+=5;
					relativePos += 5;
					flag = 1;
				}
				else if (pos+5<s.length() && s[pos] == 'i' && s[pos+1] == 'n' && s[pos+2] == 's' && s[pos+3] == 'e' 
						&& s[pos+4] == 'r' && s[pos+5] == 't' && isSeperator(s[pos+6]))
				{
					word.type = Word::_INSERT;
					pos+=6;
					relativePos += 6;
					flag = 1;
				}
				else if (pos+3<s.length() && s[pos] == 'i' && s[pos+1] == 'n' && s[pos+2] == 't' && s[pos+3] == 'o' 
						&&isSeperator(s[pos+4]))
				{
					word.type = Word::_INTO;
					pos+=4;
					relativePos += 4;
					flag = 1;
				}
				else if (pos+5<s.length() && s[pos] == 'v' && s[pos+1] == 'a' && s[pos+2] == 'l' && s[pos+3] == 'u' 
						&& s[pos+4] == 'e' && s[pos+5] == 's' && isSeperator(s[pos+6]))
				{
					word.type = Word::_VALUES;
					pos+=6;
					relativePos += 6;
					flag = 1;
				}
				else if (pos+5<s.length() && s[pos] == 'd' && s[pos+1] == 'e' && s[pos+2] == 'l' && s[pos+3] == 'e' 
						&& s[pos+4] == 't' && s[pos+5] == 'e' && isSeperator(s[pos+6]))
				{
					word.type = Word::_DELETE;
					pos+=6;
					relativePos += 6;
					flag = 1;
				}
				else if (isalpha(s[pos]) || s[pos] == '_')
				{
					temps.push_back(s[pos]);
					state = 1;//lable
				}
				else if (s[pos] == '0')
				{
					state = 2;//zero
					sum = 0;
					exp = 1;
				}
				else if (isdigit(s[pos]))
				{
					state = 3;//num
					sum = s[pos]-'0';
				}
				else if (s[pos] == '\'')
				{
					state = 6;//string
				}
				break;
			case 1://lable
				if (isalnum(s[pos]) || s[pos] == '_')
				{
					temps.push_back(s[pos]);
				}
				else if (isSeperator(s[pos]))
				{
					if (temps.length()>MAX_WORD_LEN)
					{
						sprintf(errMessage, "lable too long at Line: %d, offset: %d\n", line, relativePos);
						error = error+errMessage;
						flag = 2;
						break;
					}
					strcpy(word.str, temps.data());
					word.type = Word::_LABLE;
				    flag = 1;
				}
				else
				{
					sprintf(errMessage, "Unvalid lable at Line: %d, offset: %d\n", line, relativePos);
					error = error + errMessage;
					flag = 2;
				}
				break;
			case 2://zero
				if (s[pos] == '.')
				{
					state = 5;//double
					exp = 0.1;
				}
				else if (isSeperator(s[pos]))
				{
					word.type = Word::_NUM;
					word.value = 0;
					flag = 1;
				}
				else
				{
					sprintf(errMessage, "Unvalid num at Line: %d, offset: %d\n", line, relativePos);
					error = error + errMessage;
					flag = 2;
				}
				break;
			case 3://num
				if (s[pos] == '.')
				{
					state = 5;//double
					exp = 0.1; 	
				}
				else if (isdigit(s[pos]))
				{
					sum = sum*10 + (s[pos]-'0');
				}
				else if (isSeperator(s[pos]))
				{
					word.type = Word::_NUM;
					word.value = sum;
					flag = 1;
				}
				else
				{
					sprintf(errMessage, "unvalid num at Line: %d, offset: %d", line, relativePos);
					error = error + errMessage;
					flag = 2;
				}
				break;
			case 5://double
				if (isdigit(s[pos]))
				{
					sum = sum + exp * (s[pos]-'0');
					exp *= 0.1;
				}
				else if (isSeperator(s[pos]))
				{
					word.type = Word::_DOUBLE;
					word.value = sum;
					flag = 1;
				}
				else
				{
					sprintf(errMessage, "unvalid num at Line: %d, offset: %d", line, relativePos);
					error = error + errMessage;
					flag = 2;
				}
				break;
			case 6://string
				if (s[pos] == 0)//end of string
				{
					sprintf(errMessage, "unvalid string at Line: %d, offset: %d", line, relativePos);
					error = error + errMessage;
					flag = 2;
				}
				else if (s[pos] == '\\')
				{
					state = 7;
				}
				else if (s[pos] == '\'')
				{
					strcpy(word.str, temps.data());
					word.type = Word::_STRING;
					pos++; relativePos++;
					flag = 1;
				}
				else if (s[pos] == '\n')
				{
					sprintf(errMessage, "unvalid string at Line: %d, offset: %d", line, relativePos);
					error = error + errMessage;
					flag = 2;
				}
				else
				{
					temps.push_back(s[pos]);
				}
				break;
			case 7://escape string
				if (s[pos] == '\\')
				{
					state = 6;
					temps = temps + "\\";
				}
				else if (s[pos] == '\n')
				{
					line++;
					relativePos = 1;
					state = 6;
				}
				else if (s[pos] == '\'')
				{
					state = 6;
					temps = temps + "\'";
				}
				else
				{
					sprintf(errMessage, "unknown escape string at Line: %d, offset: %d", line, relativePos);
					error = error + errMessage;
					flag = 2;
				}
		}
		word.r = pos-1;
		if (flag) break;
		pos++; relativePos++;
	}
	return flag-1;//error 1 normal 0
}
