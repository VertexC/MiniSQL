#include "BFManager.h"
//#include "Bplus_Tree_block.h"
#include <iostream>
#include <string.h>
using namespace std;
// should try to use block file manager and leave the buffer manager
BFManager blockf_manager;

int main() {

    /*blockf_manager.createFile("test1.txt");
	const char * file_name = "test.txt";
	Bplus_Tree<string> Tree(file_name, 20, 20, 5);
	getchar();*/

	int BlockNum;
	BF_FileHandle bfh;
	BF_BlockHandle bbh;
	BFManager bfMgr;
	char *pdata = NULL;
	//remove("test1.txt");
	//bfMgr.createFile("test1.txt");
	bfMgr.openFile("test1.txt", bfh);

	/*for (int i = 0; i < 20; i++){
		bfh.AllocateBlock(bbh);
		bbh.getBlockNum(BlockNum);
		bbh.getData(pdata);
		cout << "block_num" << BlockNum << endl;
		cout << "read_in" << pdata << endl;
		if(i < 10)
			memcpy(pdata, "gooooooooooooooooooooooooood", strlen((char*)("gooooooooooooooooooooooooood")));
		else
			memcpy(pdata, "baaaaaaaaaaaaaaaaaaaaaaaaaad", strlen((char*)("baaaaaaaaaaaaaaaaaaaaaaaaaad")));
		bfh.MarkDirty(BlockNum);
		bfh.UnpinBlock(BlockNum);
	}
	bfMgr.closeFile(bfh);*/

	for (int i = 0; i < 20; i++){
		cout << "GETBLOCK" << bfh.GetThisBlock(i, bbh) << endl;
		bbh.getData(pdata);
		bbh.getBlockNum(BlockNum);
		cout << "BlockNum" << BlockNum << endl;
		cout << "read_in" << pdata << endl;
		bfh.UnpinBlock(BlockNum);
	}
	bfMgr.closeFile(bfh);
	return 0;
}

 // buffer manager
/*void test() {
	FILE *file;
	char **ppbuf = new char*;   // pointer to data
	*ppbuf = "hello";
	char **ppbuf2 = new char*;
	file = fopen("buffer.txt1", "w+");
	BF_BufferManager bufMgr(20);
	int blockNum = 5;  // assigned by you
	bufMgr.AllocateBlock(file, blockNum, ppbuf); // allocate a new block and put data of ppbuf to it
	bufMgr.GetBlock(file, blockNum, ppbuf2);  // get the data to ppbuf2
	cout << "ppbuf2=" << ppbuf2 << endl;
	// read and write: read and write the data with ppbuf
	// write back to the disk: no need, block stays in the buffer. When it is replaced, it will
	// br written to the disk. If you insisits, then call flushblocks, which will write all blocks
	// to the disks and clean the buffer.
	bufMgr.FlushBlocks(file);
}*/
