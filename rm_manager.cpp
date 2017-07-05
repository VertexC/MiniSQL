#include "RM.h"
#include "BF_FileHandle.h"
#include <math.h>
RM_Manager::RM_Manager(BFManager &new_bfm):bfm(new_bfm) {
	// have to initialize reference this way
}
int RM_Manager::CreateFile(const char *fileName, int recordSize) {
	if (recordSize <= 0 || recordSize > BF_BLOCK_SIZE)
		return -1;
	if (bfm.createFile(fileName)) // create the file using block file manager
		return -1;
	// add additional info in the file header
	int numRecord =
		(int)floor((double)(BF_BLOCK_SIZE - sizeof(RM_BlockHeader))/ ((double)recordSize + 1.0 / 8)); // 1 bit in bitmap for every record
	int bitmapSize = (numRecord % 8) ? numRecord / 8 + 1 : numRecord / 8; // make sure that the bitmap is long enough, maybe a little wasteful.
	int bitmapOffset = sizeof(RM_BlockHeader);
	if ((BF_BLOCK_SIZE - bitmapSize - bitmapOffset) / recordSize <= 0)
		return -1;  // block is not big enough
	// initialize the file header block
	BF_FileHandle fh;
	BF_BlockHandle bh;
	int blockNum;
	char *pdata;
	if (bfm.openFile(fileName, fh))   // open the file amd get the file handle
		return -1;
	if (fh.AllocateBlock(bh) ||   // allocate a new block for header
		bh.getBlockNum(blockNum) || bh.getData(pdata))
		return -1;
	RM_FileHeader *header = (RM_FileHeader *)pdata; // make the data of the block to record
	header->recordSize = recordSize;                  // the header
	header->numRecordsPerBlock = (BF_BLOCK_SIZE - bitmapSize - bitmapOffset) / recordSize;
	header->bitmapSize = bitmapSize;
	header->bitmapOffset = bitmapOffset;
	header->numBlocks = 1;  // one header block
	header->firstFree = -1;
	//if (fh.MarkDirty(blockNum) || fh.UnpinBlock(blockNum)) // mark dirty and unpin the new block
	if (fh.MarkDirty(blockNum))
		return -1;
	if (bfm.closeFile(fh)) // close the file
		return -1;
	return 0;
}
int RM_Manager::RemoveFile(const char *fileName) {
	if (bfm.removeFile(fileName))
		return -1;
	return 0;
}
int RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
	if (fileHandle.open)  // the file handle should not be opened
		return -1;
	BF_FileHandle fh;
	if (bfm.openFile(fileName, fh))
		return -1;
	BF_BlockHandle bh;
	int blockNum;
	char *pdata;
	if (fh.GetFirstBlock(bh) ||    // get the first block to set the header
		bh.getBlockNum(blockNum) || bh.getData(pdata))
		return -1;
	RM_FileHeader *header = (RM_FileHeader *)pdata;
	memcpy(&(fileHandle.header), header, sizeof(RM_FileHeader));
	fileHandle.bfh = fh;
	fileHandle.hdr_dirty = false;
	fileHandle.open = true;
	//fh.UnpinBlock(0);//unpin the first block

	return 0;
}
int RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
	BF_BlockHandle bh;
	int blockNum;
	char *pdata;
	if (fileHandle.hdr_dirty) {   // the file header is dirty, update the header and
		if (fileHandle.bfh.GetThisBlock(blockNum, bh) // mark the block dirty
			|| bh.getBlockNum(blockNum))
			return -1;
		if (bh.getData(pdata)) {
			//if (fileHandle.bfh.UnpinBlock(blockNum))
				//return -1;
			return -1;
		}
		memcpy(pdata, &fileHandle.header, sizeof(RM_FileHeader));
		//if (fileHandle.bfh.MarkDirty(blockNum) || fileHandle.bfh.UnpinBlock(blockNum))
		if (fileHandle.bfh.MarkDirty(blockNum))
			return -1;
	}
	if (bfm.closeFile(fileHandle.bfh))
		return -1;
	fileHandle.open = false;
	return 0;
}