#pragma once

#include "Interpreter.h"
#include "catlogmanager.h"
#include "BFManager.h"
#include "IndexManager.h"
#include "rm.h"
using namespace std;
using namespace Interpreter;
using namespace CatlogManager;

class API{

public:
	Index_Manager * index_m;
	RM_Manager * record_m;
	
	API();
	~API();
	//All the function needed
	void package_resolve(Package * package);//resolve the packet from intepreter

private:
	void table_create(CreateTPack * package);
	void table_drop(DropTPack * package);
	void record_insert(InsertPack * package);
	void record_delete(DeletePack * package);
	void type_trans(CreateTPack::Attri & attri, int & type);
	void index_create(CreateIPack * package);
	void index_drop(DropIPack * package);
	void selection_query(SelectPack * package);
	bool(*comparator) (void *, void *, AttrType, int, void *);
	bool satisfy(char * pData, AttrType attrType, int attrLength, int attrOffset, CompOp compOp, void * value1, void * value2);
};

