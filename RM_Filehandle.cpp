#include "RM.h"

inline bool CheckBitSet(char *bitmap, int bitmapSize, int pos) {
	if (bitmapSize < pos)
		return false;
	if (bitmap[pos / 8] & (1 << (pos % 8)))  // non-zero if the bit is set
		return true;
	else
		return false;
}
inline int SetBit(char *bitmap, int bitmapSize, int pos) { // set the pos in bitmap to 1
	if (bitmapSize < pos)
		return -1;
	bitmap[pos / 8] |= (1 << (pos % 8));
	return 0;
}
inline int ResetBit(char *bitmap, int bitmapSize, int pos) { // reset the pos in bitmap to 0
	if (bitmapSize < pos)
		return -1;
	bitmap[pos / 8] &= ~(1 << (pos % 8));
	return 0;
}
inline int GetFirstZeroBit(char *bitmap, int bitmapSize) {
	for (int i = 0; i < bitmapSize; i++) {  // just do a linear scan
		if (!(bitmap[i / 8] & (1 << (i % 8))))
			return i;
	}
	return -1;
}
inline int GetNextOneBit(char *bitmap, int bitmapSize, int start) {
	if (bitmapSize < start)
		return -1;
	for (int i = start; i < bitmapSize; i++) {  // just do a linear scan
		if ((bitmap[i / 8] & (1 << (i % 8))))
			return i;
	}
	return -1;
}
inline int ResetBitmap(char *bitmap, int bitmapSize) {
	for (int i = 0; i < bitmapSize; i++)
		bitmap[i] = bitmap[i] ^ bitmap[i]; // xor to 0
	return 0;
}



RM_FileHandle::RM_FileHandle() {
	hdr_dirty = false;
	open = false;
	header.firstFree = -1;
	header.numBlocks = 0;
}
RM_FileHandle::~RM_FileHandle() {
	open = false;
}
int RM_FileHandle::GetRec(RID &rid, RM_Record &rec) {
	//if (!isValidHandle())
		//return -1;
	int blockNum, slotNum;
	rid.GetBlockNum(blockNum); rid.GetSlotNum(slotNum);
	//if (rid.GetBlockNum(blockNum) || rid.GetSlotNum(slot)) // get the block num and slotNum
	//	return -1;
	BF_BlockHandle bh;
	if(bfh.GetThisBlock(blockNum, bh))//if ()   // get the block handle bh with blockNum
		return -1;
	char *bitmap;
	RM_BlockHeader *blockHdr;

	//if (bh.getData(pdata))    // get the data
	//	return -1;
	//blockHdr = (RM_BlockHeader *)pdata;    // take the block header from the data
	//bitmap = pdata + header.bitmapOffset;    // move to the bitmap part
	getBitmapAndBlockHdr(bh, bitmap, blockHdr);
	// check bit set? whether the record really exits? TODO

	//set the RID record with pdata and size
	rec.SetRecord(rid, bitmap + header.bitmapSize + slotNum * header.recordSize, header.recordSize);


	//bfh.UnpinBlock(blockNum);
	return 0;
}

int RM_FileHandle::InsertRec(char *pdata, RID &rid) {
	if (!pdata)
		return -1;
	BF_BlockHandle bh;
	int blockNum;
	char *bitmap;
	RM_BlockHeader *blockHdr;
	if (header.firstFree == -1) {  // no more free block, then allocate a new one
		bfh.AllocateBlock(bh);
		bh.getBlockNum(blockNum);
		getBitmapAndBlockHdr(bh, bitmap, blockHdr);
		blockHdr->nextFree = header.firstFree; // initialize block header
		blockHdr->numRecords = 0;
		ResetBitmap(bitmap, header.bitmapSize); // reset bitmap to all 0
		header.numBlocks++;
		header.firstFree = blockNum;  // link the new block to the free list
	}
	else {
		blockNum = header.firstFree;
		bfh.GetThisBlock(blockNum, bh);
	}

	int slot;
	getBitmapAndBlockHdr(bh, bitmap, blockHdr);

	slot = GetFirstZeroBit(bitmap, header.numRecordsPerBlock); // get and the first free slot
	SetBit(bitmap, header.numRecordsPerBlock, slot);     // set it to occupied
	memcpy(bitmap + header.bitmapSize + slot * header.recordSize, pdata, header.recordSize);
	(blockHdr->numRecords)++;
	rid = RID(blockNum, slot);

	if (blockHdr->numRecords == header.numRecordsPerBlock) // if the block is full
		header.firstFree = blockHdr->nextFree;

	bfh.MarkDirty(blockNum); //bfh.UnpinBlock(blockNum);
	return 0;
}

int RM_FileHandle::DeleteRec(RID &rid){
	//if (!isValidFH())
		//return -1;

	int rc = 0;

	// Retrieve page and slot number from the RID
	int blockNum;
	int slotNum;
	rid.GetBlockNum(blockNum); rid.GetSlotNum(slotNum);

	// Get this page, its bitmap, and its header
	BF_BlockHandle bh;
	if ((rc = bfh.GetThisBlock(blockNum, bh)))
		return (rc);
	char *bitmap;
	RM_BlockHeader *blockHdr;
	getBitmapAndBlockHdr(bh, bitmap, blockHdr);

	// Check if there really exists a record here according to the header
	bool recordExists;
	recordExists = CheckBitSet(bitmap, header.numRecordsPerBlock, slotNum);
	if (!recordExists) {
		//bfh.UnpinBlock(blockNum);
		return -1;
	}

	// Reset the bit to indicate a record deletion
	ResetBit(bitmap, header.numRecordsPerBlock, slotNum);
	blockHdr->numRecords--;
	// update the free list if the block changes from full to not full
	if (blockHdr->numRecords == header.numRecordsPerBlock - 1) {
		blockHdr->nextFree = header.firstFree;
		header.firstFree = blockNum;
	}

	bfh.MarkDirty(blockNum); //bfh.UnpinBlock(blockNum);
	return 0;
}

int RM_FileHandle::UpdateRec(RM_Record &rec){
	// check FH, get RID, get blockNum & slot, get block, get block header: bitmap and data
	RID rid;
	rec.GetRid(rid);
	int blockNum, slotNum;
	rid.GetBlockNum(blockNum); rid.GetSlotNum(slotNum);
	BF_BlockHandle bh;
	char *bitmap;
	RM_BlockHeader *blockHdr;
	getBitmapAndBlockHdr(bh, bitmap, blockHdr);
	// check bitmap to find the slot
	bool recordExists = CheckBitSet(bitmap, header.numRecordsPerBlock, slotNum);
	if (!recordExists) {
		//bfh.UnpinBlock(blockNum);
		return -1;
	}
	// update
	char * recData;
	rec.GetData(recData);
	memcpy(bitmap + header.bitmapSize + slotNum * header.recordSize,
		recData, header.recordSize);
	bfh.MarkDirty(blockNum); //bfh.UnpinBlock(blockNum);
	return 0;
}

//int RM_FileHandle::ForceBlocks(int blockNum){
//	bfh.ForceBlocks(blockNum);
//	return 0;
//}

int RM_FileHandle::GetNextRec(int blockNum, int slotNum, BF_BlockHandle &bh, RM_Record &rec, bool nextMode) { // get the next record in the block
	char *bitmap;
	RM_BlockHeader *blockHdr;
	int pos, nextBlock = blockNum;
	if (nextMode) {
		while (true) {
			if (bfh.GetNextBlock(nextBlock, bh))
				return -1;
			bh.getBlockNum(nextBlock);  // assign the new block number to update nextBlock
			getBitmapAndBlockHdr(bh, bitmap, blockHdr);
			if ((pos = GetNextOneBit(bitmap, header.numRecordsPerBlock, 0)) != -1)
				break;  // find one block with records
			//bfh.UnpinBlock(nextBlock); // unpin the block if no records
		}
	}
	else {
		getBitmapAndBlockHdr(bh, bitmap, blockHdr); // get from the current bh
		if (slotNum >= blockHdr->numRecords)
			return -1;
		if ((pos = GetNextOneBit(bitmap, header.numRecordsPerBlock, slotNum + 1)) == -1)
			return -1;
	}

	RID rid(nextBlock, pos);
	rec.SetRecord(rid, bitmap + header.bitmapSize + header.recordSize * pos, header.recordSize);
	return 0;
}


inline int RM_FileHandle::getBitmapAndBlockHdr(BF_BlockHandle bh, char *&bitmap, RM_BlockHeader *&blockHdr) {
	char *pdata;
	if (bh.getData(pdata))    // get the data
		return -1;
	blockHdr = (RM_BlockHeader *)pdata;    // take the block header from the data
	bitmap = pdata + header.bitmapOffset;    // move to the bitmap part
	return 0;
}
