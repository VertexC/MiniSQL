#ifndef INTERPRETER_INTERNAL_H_
#define INTERPRETER_INTERNAL_H_
#include "interpreter.h"
#include <string>

//for parsing command
struct Word
{
	enum{_AND=1, _CHAR, _CREATE, _DROP, _EXECFILE, _FLOAT, _FROM
		,_INDEX, _INT, _KEY, _ON, _PRIMARY, _QUIT, _SELECT, _TABLE
		,_UNIQUE, _WHERE, _LABLE, _NUM, _DOUBLE, _STRING, _SEMI, _COMMA
		,_LPAR, _RPAR, _EQU, _LES, _GRE, _NOTEQU, _LESEQU, _GREEQU, _STAR
	    ,_INSERT, _INTO, _VALUES, _DELETE};//word type
	int type;
	int line;
	int relativePos;//the relative position in one line
	int l, r;
	double value;
	char str[MAX_WORD_LEN+1];
};

namespace _Interpreter
{
	bool isSeperator(char c);
	int getWord(const std::string &s, unsigned int &pos, int &line, int &relativePos, Word &word, std::string &error);
	bool scan(const std::string &s, std::vector<Word> &words, std::string &error);
	Package *getCommand(const std::vector<Word> &words, std::string &error);
	
	Package *getCreateTable(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getCreateIndex(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getCreate(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getDropTable(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getDropIndex(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getDrop(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getSelect(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getInsert(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getDelete(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getExe(const std::vector<Word> &words, unsigned int &pos, std::string &error);
	Package *getQuit(const std::vector<Word> &words, unsigned int &pos, std::string &error);
}

#endif