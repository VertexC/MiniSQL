#pragma once

/*
 * Block file manager handles the creation, deletion, opening, and closing of block files
 *  as well as the management of the buffer pool
 */
//only one BFManager in the program
#include "BF_BufferManager.h"
#include "BF_FileHandle.h"
class BFManager{ // block file manager
public:
	BFManager();
	~BFManager();
	//create but not open
	int createFile(const char *fileName);
	int removeFile(const char *fileName);
	int openFile(const char *fileName, BF_FileHandle &fileHandle);
	int closeFile(BF_FileHandle &fileHandle);
private:
	static int fno_now;
	BF_BufferManager *buffer_pool;
};

struct BF_FileHdr {
	int firstFree;     // first free block in the linked list
	int numBlocks;      // # of blocks in the file
};

extern BFManager bfm;
