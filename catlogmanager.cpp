#include "catlogmanager.h"
#include "rm.h"
#include "interpreter.h"
#include <cstring>
#include <iostream>

void CatlogManager::init()
{
	using namespace CatlogManager;
	RM_Manager rmMgr(bfm);//global BFManager bfm
	RM_FileHandle fhd;

	rmMgr.CreateFile("sys_TableRDs", sizeof(TableRD));
	rmMgr.CreateFile("sys_AttriRDs", sizeof(AttriRD));
	rmMgr.CreateFile("sys_IndexRDs", sizeof(IndexRD));
}

bool CatlogManager::getTable(char *table, char *&table_file)
{
	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd;
	RM_FileScan records;
	RM_Record rec;
	TableRD *rd;
	char *p;
	bool flag = false;

	rmMgr.OpenFile("sys_TableRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_TABLE_LEN+1, 0, CompOp::EQ, table, table);
	if(!records.GetNextRec(rec))
	{
		rec.GetData(p);
		rd = reinterpret_cast<TableRD *>(p);
		table_file = new char[strlen(rd->table_file)+1];
		std::strcpy(table_file, rd->table_file);
		flag = true;
	}
	records.CloseScan();
	rmMgr.CloseFile(fhd);
	return flag;
}

bool CatlogManager::getAttribute(char *table, const char *attri, Interpreter::CreateTPack::Attri &attr)
{
	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd;
	RM_FileScan records;
	RM_Record rec;
	AttriRD *rd;
	char *p;
	bool flag = false;

	rmMgr.OpenFile("sys_AttriRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_TABLE_LEN+1, 0, CompOp::EQ, table, table);
	while(!records.GetNextRec(rec))
	{
		rec.GetData(p);
		rd = reinterpret_cast<AttriRD *>(p);
		if (std::strcmp(rd->attri.name,attri) == 0)
		{
			flag = true;
			attr = rd->attri;
		}
	}
	records.CloseScan();
	rmMgr.CloseFile(fhd);
	return flag;
}

bool CatlogManager::getAttribute(char *table, std::vector<Interpreter::CreateTPack::Attri> &attrs)
{
	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd;
	RM_FileScan records;
	RM_Record rec;
	AttriRD *rd;
	char *p;
	bool flag = false;

	rmMgr.OpenFile("sys_AttriRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_TABLE_LEN+1, 0, CompOp::EQ, table, table);
	while(!records.GetNextRec(rec))
	{
		rec.GetData(p);
		rd = reinterpret_cast<AttriRD *>(p);
		attrs.push_back(rd->attri);
		flag = true;

	}
	records.CloseScan();
	rmMgr.CloseFile(fhd);
	return flag;
}

bool CatlogManager::getIndex(char *table, const char *attri, char * &index_file)
{
	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd;
	RM_FileScan records;
	RM_Record rec;
	IndexRD *id;
	char *p;
	bool flag = false;


	if (rmMgr.OpenFile("sys_IndexRDs",  fhd))return false;
	if (records.OpenScan(fhd, STRING_, MAX_TABLE_LEN+1, 0, CompOp::EQ, table, table))return false;
	while(!records.GetNextRec(rec))
	{
		if (rec.GetData(p))return false;
		//std::cout<<p<<endl;
		id = reinterpret_cast<IndexRD *>(p);
		//std::cout<<id->table<<endl;
		//std::cout<<id->attri<<endl;
		if (std::strcmp(id->attri,attri) == 0)
		{
			flag = true;
			index_file = new char[strlen(id->index_file)+1];
			std::strcpy(index_file, id->index_file);
		}
	}
	if (records.CloseScan())return false;
	if (rmMgr.CloseFile(fhd))return false;
	return flag;
}

bool CatlogManager::getIndex(char *index, char *&index_file)
{
	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileScan records;
	RM_FileHandle fhd;
	RM_Record rec;
	IndexRD *id;
	char * p;
	bool flag = false;

	rmMgr.OpenFile("sys_IndexRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_INDEX_LEN+1, Interpreter::MAX_TABLE_LEN+1, CompOp::EQ, index, index);
	if(!records.GetNextRec(rec)) 
	{
		rec.GetData(p);
		id = reinterpret_cast<IndexRD *>(p);
		index_file = new char[strlen(id->index_file)+1];
		strcpy(index_file,id->index_file);
		flag = true;
	}
	records.CloseScan();
	rmMgr.CloseFile(fhd);
	return flag;
}

bool CatlogManager::existIndex(char * index)
{
	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileScan records;
	RM_FileHandle fhd;
	RM_Record rec;
	bool flag = false;

	rmMgr.OpenFile("sys_IndexRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_INDEX_LEN+1, Interpreter::MAX_TABLE_LEN+1, CompOp::EQ, index, index);
	if(records.GetNextRec(rec)) 
		flag = false;
	else 
		flag = true;
	records.CloseScan();
	rmMgr.CloseFile(fhd);
	return flag;
}

bool CatlogManager::createTable(Interpreter::CreateTPack *package)
{
	char *table_file = NULL;
	if (getTable(package->table,table_file))
	{
		delete table_file;
		return false;
	}

	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd1,fhd2,fhd3;
	TableRD *trd;
	AttriRD *ard;
	IndexRD *ird;
	RID rid;

	rmMgr.OpenFile("sys_TableRDs",  fhd1);
	trd = new TableRD();
	strcpy(trd->table, package->table);
	strcpy(trd->table_file,package->table);
	strcat(trd->table_file,"_tab");
	fhd1.InsertRec(reinterpret_cast<char *>(trd), rid);
	rmMgr.CloseFile(fhd1);

	rmMgr.OpenFile("sys_AttriRDs",  fhd2);
	std::vector<CreateTPack::Attri>::iterator it;
	for (it = package->attri.begin(); it != package->attri.end(); it++)
	{
		ard = new AttriRD();
		strcpy(ard->table, package->table);
		ard->attri = *it;
		fhd2.InsertRec(reinterpret_cast<char*>(ard), rid);
	}
	rmMgr.CloseFile(fhd2);

	rmMgr.OpenFile("sys_IndexRDs", fhd3);
	ird = new IndexRD();
	strcpy(ird->table, package->table);
	strcpy(ird->attri, package->PK);
	strcpy(ird->index, "");
	strcpy(ird->index_file,package->table);
	strcat(ird->index_file, "_pk");
	ird->type = CatlogManager::IndexType::B;
	fhd3.InsertRec(reinterpret_cast<char*>(ird), rid);
	rmMgr.CloseFile(fhd3);
	return true;
}

bool CatlogManager::createIndex(Interpreter::CreateIPack *package)
{
	char *index_file = NULL;
	if (getIndex(package->table,package->attri,index_file))
	{
		delete index_file;
		return false;
	}
	if (existIndex(package->index))
	{
		return false;
	}

	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd;
	IndexRD *ird;
	RID rid;

	rmMgr.OpenFile("sys_IndexRDs", fhd);
	ird = new IndexRD();
	strcpy(ird->table, package->table);
	strcpy(ird->attri, package->attri);
	strcpy(ird->index, package->index);
	strcpy(ird->index_file, package->index);
	strcat(ird->index_file, "_ind");
	ird->type = CatlogManager::IndexType::B;
	fhd.InsertRec(reinterpret_cast<char*>(ird), rid);
	rmMgr.CloseFile(fhd);
	return true;
}

bool CatlogManager::dropTable(char * table)
{
	char *table_file;
	if (!getTable(table,table_file))
	{
		delete table_file;
		return false;
	}

	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd;
	RM_FileScan records;
	RM_Record rec;
	RID rid;

	rmMgr.OpenFile("sys_TableRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_TABLE_LEN+1, 0, CompOp::EQ, table, table);
	records.GetNextRec(rec);
	rec.GetRid(rid);
	fhd.DeleteRec(rid);
	records.CloseScan();
	rmMgr.CloseFile(fhd);

	rmMgr.OpenFile("sys_AttriRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_TABLE_LEN+1, 0, CompOp::EQ, table, table);
	while(!records.GetNextRec(rec))
	{
		rec.GetRid(rid);
		fhd.DeleteRec(rid);
	}
	records.CloseScan();
	rmMgr.CloseFile(fhd);

	rmMgr.OpenFile("sys_IndexRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_TABLE_LEN+1, 0, CompOp::EQ, table, table);
	while(!records.GetNextRec(rec))
	{
		rec.GetRid(rid);
		fhd.DeleteRec(rid);
	}
	records.CloseScan();
	rmMgr.CloseFile(fhd);

	return true;
}

bool CatlogManager::dropIndex(char * index)
{
	if (!existIndex(index))
	{
		return false;
	}

	using namespace Interpreter;
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd;
	RM_FileScan records;
	RM_Record rec;
	RID rid;

	rmMgr.OpenFile("sys_IndexRDs",  fhd);
	records.OpenScan(fhd, STRING_, MAX_INDEX_LEN+1, Interpreter::MAX_TABLE_LEN+1, CompOp::EQ, index, index);
	while(!records.GetNextRec(rec))
	{
		rec.GetRid(rid);
		fhd.DeleteRec(rid);
	}
	records.CloseScan();
	rmMgr.CloseFile(fhd);

	return true;
}
