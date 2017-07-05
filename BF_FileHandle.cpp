#include "BF_FileHandle.h"
#include "BF_internal.h"
#include <cstdio>
#include <cstring>
using namespace std;
BF_FileHandle::BF_FileHandle() {
	fileOpen = false;
	bufMgr = NULL;
	fno = 0;
}

BF_FileHandle::~BF_FileHandle()
{
	if (!fileOpen)
		return;

	FILE* file;
	file = fopen(fno_name[fno].data(),"rb+");
	if (!file) return;

	if (hdrDirtyFlag) { // the file header is changed
		if (fseek(file, 0, SEEK_SET)) // set to the beginning
		{
			fclose(file);
			return;
		}
		int c;
		if ((c=fwrite(&header, sizeof(char), sizeof(fileHeader), file)) != sizeof(fileHeader))
		{
			fclose(file);
			return;   // write the file header
		}
		hdrDirtyFlag = false;
	}
	fclose(file);
}

int BF_FileHandle::GetThisBlock(int current, BF_BlockHandle &blockHandle) {
	char *blockBuf; // ptr to data of the block in the buffer pool
	if (!fileOpen)
		return -1;
	if (!isValidBlockNum(current))
		return -1;
	if ((bufMgr->GetBlock(fno, current, blockBuf))) // get the block from the buffer manager
		return -1;    // return non-zero when error occurs
	if (((blockHeader *)blockBuf)->nextFree == -2) { // the block is used
		blockHandle.blockNum = current;
		blockHandle.blockData = blockBuf + sizeof(blockHeader); // the address of data is
		return 0;															// next to the address of block header
	}

	//UnpinBlock(current);  // current block is free
	return -1;
}

int BF_FileHandle::GetNextBlock(int current, BF_BlockHandle &blockHandle) {
	if (!fileOpen)
		return -1; // error if file is not opened
	if (current != -1 && !isValidBlockNum(current))
		return -1; // error if invalid current block num
	current++; // get to the next position
	for (; current < header.numBlock; current++) {
		if (!GetThisBlock(current, blockHandle))
			return 0;  // find the next in used block
	}
	return -1; // can not find the next block
}

int BF_FileHandle::GetPrevBlock(int current, BF_BlockHandle &blockHandle) {
	if (!fileOpen)
		return -1; // error if file is not opened
	if (current != header.numBlock && !isValidBlockNum(current))
		return -1;  // error if invalid current block num
	current--; // get to the prev position
	for (; current >= 0; current--) {
		if (!GetThisBlock(current, blockHandle))
			return 0;  // find the next in used block
	}
	return -1; // can not find the next block
}

int BF_FileHandle::GetFirstBlock(BF_BlockHandle &blockHandle) {
	return GetNextBlock(-1, blockHandle);
}

int BF_FileHandle::GetLastBlock(BF_BlockHandle &blockHandle) {
	return GetPrevBlock(header.numBlock, blockHandle);
}

int BF_FileHandle::AllocateBlock(BF_BlockHandle &blockHandle) {
	int blockNum;  // new block number
	char *blockBuf_ptr; // address of the block in the buffer pool

	if (!fileOpen)
		return -1;

	if (header.firstFree != -1) { // the free list is not empty
		blockNum = header.firstFree; // give the first free block as a new block
		if (bufMgr->GetBlock(fno, blockNum, blockBuf_ptr))
			return -1;  // error getting the block
		header.firstFree = ((blockHeader *)blockBuf_ptr)->nextFree; // update the first free block in the header
	}
	else { // the free list is empty, need to actually allocate a new block
		blockNum = header.numBlock;
		if (bufMgr->AllocateBlock(fno, blockNum, blockBuf_ptr))
			return -1; // error allocating a new block
		header.numBlock++;
	}

	hdrDirtyFlag = true;
	((blockHeader *)blockBuf_ptr)->nextFree = -2; // this block is used

	memset(blockBuf_ptr + sizeof(blockHeader), 0, BF_BLOCK_SIZE); // set data to 0
	if (MarkDirty(blockNum))
		return -1;  // have error mark new block dirty
	blockHandle.blockData = blockBuf_ptr + sizeof(blockHeader);
	blockHandle.blockNum = blockNum;
	return 0;
}

int BF_FileHandle::DisposeBlock(int blockNum) {
	char *blockBuf;
	if (!fileOpen)
		return -1;
	if (!isValidBlockNum(blockNum))
		return -1;

	if (bufMgr->GetBlock(fno, blockNum, blockBuf)) // get the block and it must be used
		return -1;
	if (((blockHeader *)blockBuf)->nextFree != -2) {  // if the block is not used
		//if (UnpinBlock(blockNum))
			//return -1; // error unpin the block
		return -1;
	}

	// put the block in the free list
	((blockHeader *)blockBuf)->nextFree = header.firstFree;
	header.firstFree = blockNum;
	hdrDirtyFlag = true;
	if (MarkDirty(blockNum))
		return -1;
	//if (UnpinBlock(blockNum))
		//return -1;
	return 0;
}

int BF_FileHandle::MarkDirty(int blockNum) {
	if (!fileOpen)
		return -1;
	if (!isValidBlockNum(blockNum))
		return -1;
	return bufMgr->MarkDirty(fno, blockNum);
}

int BF_FileHandle::PinBlock(int blockNum)
{
	if (!fileOpen)
		return -1;
	if (!isValidBlockNum(blockNum))
		return -1;
	return bufMgr->PinBlock(fno, blockNum);
}

int BF_FileHandle::UnpinBlock(int blockNum) {
	if (!fileOpen)
		return -1;
	if (!isValidBlockNum(blockNum))
		return -1;
	return bufMgr->UnpinBlock(fno, blockNum);
}

int BF_FileHandle::FlushBlocks() {
	if (!fileOpen)
		return -1;

	FILE* file;
	file = fopen(fno_name[fno].data(),"rb+");
	if (!file) return -1;

	if (hdrDirtyFlag) { // the file header is changed
		if (fseek(file, 0, SEEK_SET)) // set to the beginning
		{
			fclose(file);
			return -1;
		}
		int c;
		if ((c=fwrite(&header, sizeof(char), sizeof(fileHeader), file)) != sizeof(fileHeader))
		{
			fclose(file);
			return -1;   // write the file header
		}
		hdrDirtyFlag = false;
	}
	fclose(file);
	return bufMgr->FlushBlocks(fno);
}

int BF_FileHandle::ForceBlocks(int blockNum)
{
	if (!fileOpen)
		return -1;
	FILE* file;
	file = fopen(fno_name[fno].data(),"rb+");
	if (!file) return -1;

	if (hdrDirtyFlag) { // the file header is changed
		if (fseek(file, 0, SEEK_SET)) // set to the beginning
		{
			fclose(file);
			return -1;
		}
		int c;
		if ((c=fwrite(&header, sizeof(char), sizeof(fileHeader), file)) != sizeof(fileHeader))
		{
			fclose(file);
			return -1;   // write the file header
		}
		hdrDirtyFlag = false;
	}
	fclose(file);
	return bufMgr->ForceBlocks(fno);
}

int BF_FileHandle::isValidBlockNum(int blockNum) {
	return fileOpen && blockNum >= 0 && blockNum < header.numBlock;
}

BF_BlockHandle::BF_BlockHandle() {
	blockNum = -1;
	blockData = NULL;
}

int BF_BlockHandle::getData(char *&data) {
	if (!blockData)
		return -1;   // error if the block is unpinned
	data = blockData;
	return 0;
}

int BF_BlockHandle::getBlockNum(int &num) {
	if (!blockData)
		return -1; // error if the block is unpinned
	num = blockNum;
	return 0;
}
