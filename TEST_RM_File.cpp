#include "RM.h"
#include "RM_RID.h"
#include <iostream>
using namespace std;
int main(){
	BFManager bfm;  // have to associate a BFManager, openFile and closeFile depend on the it
	RM_Manager rmMgr(bfm);
	RM_FileHandle fhd;
	RID rid;
	char *pdata = "hello";  // ptr to data, char only means byte
	char *pdata2;
	int recordSize = 10;
    //remove("test.txt");
	//rmMgr.CreateFile("test.txt", recordSize); // create a file in which the records are of recordSize
	//rmMgr.CreateFile("test2.txt", recordSize);
	//rmMgr.CreateFile("test3.txt", recordSize);
	//rmMgr.OpenFile("test.txt", fhd); // get a rm filehandle fhd
	if (rmMgr.OpenFile("test.txt", fhd)) {
		rmMgr.CreateFile("test.txt", 50);
	}
	rmMgr.OpenFile("test2.txt", fhd);
	rmMgr.OpenFile("test3.txt", fhd);
	int blockNum, slotNum; // files are consist of blocks(each has a blockNum), blocks are consists of records(each has a slotNum)
	fhd.InsertRec(pdata, rid); // data(pdata) is written to the record identified by rid
	RM_Record rec;
	fhd.GetRec(rid, rec);
	rec.GetData(pdata2);
	cout << pdata2 << endl;
	return 0;
}