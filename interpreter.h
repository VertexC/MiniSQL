#ifndef INTERPRETER_H_
#define INTERPRETER_H_
#include <vector>
#include <string>

//MAX_WORD_LEN should be greater or equal than others
namespace Interpreter
{
	const int MAX_ATTRI_LEN = 255;
	const int MAX_CHAR_LEN = 255;
	const int MAX_TABLE_LEN = 255;
	const int MAX_INDEX_LEN = 255;
	const int MAX_FILE_LEN = 255;
	const int MAX_WORD_LEN = 255;
	struct Package;
	struct CreateTPack;
	struct CreateIPack;
	struct DropTPack;
	struct DropIPack;
	struct SelectPack;
	struct InsertPack;
	struct DeletePack;
	struct ExePack;

	Package *interpreter(const std::string &command, std::string &error);
}

using namespace Interpreter;

/*
*this is an abstract struct
*/
struct Interpreter::Package
{
	enum{LES_ = 1, EQU_ = 0, GRE_ = 2, LESEQU_ = 3, GREEQU_ = 4, NOTEQU_ = 5};//condition op
		//<        =     >     <=       >=       <>
	enum{INT_, FLOAT_, CHAR_};//attribute type

	enum{CREATET_PACK = 1, CREATEI_PACK, DROPT_PACK,
		DROPI_PACK, SELECT_PACK, INSERT_PACK,
		DELETE_PACK, EXECFILE_PACK, QUIT_PACK};//package type

	int type;
	virtual ~Package(){}
};

/*
*create table package:information for create table
*/
struct Interpreter::CreateTPack:public Package//create table pack
{
	/*
	*an attribute in create command
	*/
	struct Attri
	{
		char name[MAX_ATTRI_LEN+1];//attribute name
		int type;//attribute type
		int size;
		bool unique;//1 unique 0 non unique
	};
//*******************************************
	char table[MAX_TABLE_LEN+1];//table name
	std::vector<Attri> attri;//attributes
	char PK[MAX_ATTRI_LEN+1];//primary key name
};

//drop table package
struct Interpreter::DropTPack:public Package//drop table pack
{
	char table[MAX_TABLE_LEN+1];
};

//create index package
struct Interpreter::CreateIPack:public Package
{
	char index[MAX_INDEX_LEN+1];//index name
	char table[MAX_TABLE_LEN+1];//table name
	char attri[MAX_ATTRI_LEN+1];//attribute name
};

//drop index package
struct Interpreter::DropIPack:public Package
{
	char index[MAX_INDEX_LEN+1];//index name
};

//select package
struct Interpreter::SelectPack:public Package
{
	/*
	*one select condition
	*/
	struct Condition
	{
		char attri[MAX_ATTRI_LEN+1];
		int op;
		int type;
		union
		{
			int i;
			double f;
			char c[MAX_CHAR_LEN+1];
		} value;
	};
//*************************************
	std::vector<char*> attri;
	char table[MAX_TABLE_LEN+1];
	std::vector<Condition> condition;//no condition is possible
	~SelectPack()
	{
		for (std::vector<char*>::iterator it = attri.begin(); it!= attri.end(); it++)
		{
			delete[] *it;
		}
	}
};

//insert package
struct Interpreter::InsertPack:public Package
{
	struct Value
	{
		int type;
		int size;
		union
		{
			int i;
			double f;
			char c[MAX_CHAR_LEN+1];
		}value;
	};
//*************************************
	char table[MAX_TABLE_LEN+1];//table name
	std::vector<Value> value;
};

//delete package
struct Interpreter::DeletePack:public Package
{
	/*
	*one select condition
	*/
	struct Condition
	{
		char attri[MAX_ATTRI_LEN+1];
		int op;
		int type;
		union
		{
			int i;
			double f;
			char c[MAX_CHAR_LEN+1];
		} value;
	};
//******************************************
	char table[MAX_TABLE_LEN+1];//table name
	std::vector<Condition> condition;//no condition is possible
};

//exefile package
struct Interpreter::ExePack:public Package
{
	char file[MAX_FILE_LEN+1];//file name
};
#endif