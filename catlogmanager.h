#pragma once
#ifndef CATLOGMANAGER_H_
#define CATLOGMANAGER_H_
#include "interpreter.h"
#include "rm.h"

//主索引的名字为空
//主索引对应的文件为 表名_pk
//表对应的文件为 表名_tab
//索引对应的文件为 索引名_ind

// MetaData struct cotains 1 or more records
// AttibuteMetaData may contains more than one meta data of attribute
namespace CatlogManager
{
	using namespace Interpreter;
	/*
	*have to be called every time the first time program start
	*/
	void init();

	/*
	 *get tablefile name by table name
	 *table_file should not be newed before calling this function
	 *table_file should be freed after calling this function
	 *true table exist false table doesn't exist
	 */
	bool getTable(char *table, char *&table_file);

	/*
	 *true attribute exist false attribute doens't exist
	 */
	bool getAttribute(char *table, const char *attri, Interpreter::CreateTPack::Attri &attr);
	/*
	 *true attribute exist false attribute doesn't exist
	 *return attributes for given table name
	 */
	bool getAttribute(char *table, std::vector<Interpreter::CreateTPack::Attri> &attrs);

	/*
	 *get indexfile name by table and attribute name
	 *index_file should not be newed before calling this function
	 *index_file should be freed after calling this function
	 *true index exist false index doesn't exist
	 */
	bool getIndex(char *table, const char *attri, char * &index_file);
	/*
	 *get indexfile name by index name
	 */
	bool getIndex(char *index, char *&index_file);
	//if the index with the given index name exist
	bool existIndex(char * index);
	
	/*
	 *table files and index files should be created after calling these functions
	 */
	bool createTable(Interpreter::CreateTPack *package);
	bool createIndex(Interpreter::CreateIPack *package);

	/*
	 *table files and index files should be deleted on disk first
	 */
	bool dropTable(char * table);

	/*
	 *table files and index files should be deleted on disk first
	 */
	bool dropIndex(char *index);

	struct TableRD;
	struct AttriRD;
	struct IndexRD;
	enum IndexType{B = 1};//index type
};

struct CatlogManager::TableRD
{
	char table[Interpreter::MAX_TABLE_LEN+1];//must be placed first
	char table_file[Interpreter::MAX_TABLE_LEN+4+1];//add length of postfix "_tab"
};

struct CatlogManager::AttriRD
{
	char table[Interpreter::MAX_TABLE_LEN+1];//must be placed first
	Interpreter::CreateTPack::Attri attri;
};

struct CatlogManager::IndexRD
{
	char table[Interpreter::MAX_TABLE_LEN+1];//must be placed first
	char index[Interpreter::MAX_INDEX_LEN+1];
	char attri[Interpreter::MAX_ATTRI_LEN+1];
	char index_file[Interpreter::MAX_INDEX_LEN+4+1];//add post fix length "_pk" or "_ind"
	int type;
};

#endif