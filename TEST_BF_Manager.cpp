#include "BFManager.h"
#include <iostream>
using namespace std;
int main() {
	BFManager bfMgr;
	BF_FileHandle bfh;
	BF_BlockHandle bbh;
	bfMgr.createFile("test1.txt");
	bfMgr.openFile("test1.txt", bfh);
	for (int i = 0; i < 3; i++)
		bfh.AllocateBlock(bbh);
	bfh.
	bfh.GetThisBlock(2, bbh);
	int blockNum;
	bbh.getBlockNum(blockNum);
	cout << "blockNum is " << blockNum << endl;
	char *pdata;
	bbh.getData(pdata);
	printf("data is %s\n", pdata);
	return 0;
}