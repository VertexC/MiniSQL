#pragma once
/*
 * FileHandle can allocate, disposal and modify a block in a block file
 */
//remember to unpin blocks
#include <cstdio>
#include "BF_BufferManager.h"

struct fileHeader {
	int firstFree;
	int numBlock;
};

class BF_BlockHandle {
public:
	BF_BlockHandle();
	int getData(char *&blockData);
	int getBlockNum(int &blockNum);
private:
	int blockNum;
	char *blockData;
friend class BF_FileHandle;
};

class BF_FileHandle {
public:
	BF_FileHandle();
	~BF_FileHandle();

	// Get the first block
	int GetFirstBlock(BF_BlockHandle &blockHandle);
	// Get the next block after current
	int GetNextBlock(int current, BF_BlockHandle &blockHandle);
	// Get a specific block
	int GetThisBlock(int BlockNum, BF_BlockHandle &blockHandle);
	// Get the last block
	int GetLastBlock(BF_BlockHandle &blockHandle);
	// Get the prev block after current
	int GetPrevBlock(int current, BF_BlockHandle &blockHandle) ;

	int AllocateBlock(BF_BlockHandle &blockHandle);    // Allocate a new block
	int DisposeBlock(int BlockNum);              // Dispose of a block
	int MarkDirty(int BlockNum);        // Mark block as dirty
	int UnpinBlock(int BlockNum);        // Unpin the block
	int PinBlock(int BlockNum);			//pin the block

	// Flush all blocks from buffer pool.  Will write dirty blocks to disk. 
	//wil remove unpined blocks from the buffer pool
	int FlushBlocks();

	// Force a block to disk will not remove from the buffer pool
	int ForceBlocks(int blockNum);
private:
	int isValidBlockNum(int blockNum);
	BF_BufferManager *bufMgr;
	fileHeader header;
	bool fileOpen;
	bool hdrDirtyFlag;
	FILENO fno;
friend class BFManager;
};
