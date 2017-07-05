#pragma once

//IndexManager.h 
//Provide functions to manage the indexe
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include "Bplus_Tree_block.h"
//#include "windows.h"

//------------To manage all the indexes--------------------//
class Index_Manager{
private:
	//BFManager blockf_manager;
	/*typedef map<const char *, Bplus_Tree<int> *> int_index_map;
	typedef map<const char *, Bplus_Tree<float> *> float_index_map;
	typedef map<const char *, Bplus_Tree<string> *> string_index_map;*/
	
	//int static const FLOAT_TYPE = -1;
	//int static const INT_TYPE = 0;
	//the size of string type is > 0
	//API * api;// to call api functions
	//int_index_map int_index;
	//float_index_map float_index;
	//string_index_map string_index;
	int int_key;
	float float_key;
	string string_key;
public:
	Index_Manager(){}
	~Index_Manager(){}

	void set_key(int type, string key);//set the value in my key
	int get_degree(int type);
	int get_key_size(int type);
	void create_Index(const char * filename, int type);
	void drop_Index(const char * filename);
	//offset is the adress of tuple in all records
	offset search_Index(const char * filename, string key, int type);
	void delete_Index(const char * filename, string key, int type);
	void delete_Index(const char * filename, offset address, int type);
	bool insert_Index(const char * filename, string key, offset address, int type);
	void Index_search(const char * filename, void * compare_value, AttrType attrType, CompOp compOp, vector<RID> & result_rid);
};

