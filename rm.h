#pragma once
#include "BFManager.h"
#include "RM_RID.h"
#include "BF_FileHandle.h"
#define ALL_BLOCKS = -2
const int MAXSTRINGLEN = 255;
struct RM_BlockHeader {
	int nextFree;
	int numRecords;
};
struct RM_FileHeader {
	int numBlocks;
	int numRecordsPerBlock;
	int recordSize;
	int firstFree;
	int bitmapOffset;
	int bitmapSize;
};
class RM_Record {
public:
	RM_Record();                     // Constructor
	~RM_Record();                     // Destructor

	RM_Record(RM_Record& r)
	{
		rid = r.rid;
		size = r.size;
		data = new char[r.size];
		memcpy(data,r.data,r.size);
	}

	RM_Record& operator=(const RM_Record& r)
	{
		rid = r.rid;
		size = r.size;
		data = new char[r.size];
		memcpy(data,r.data,r.size);
		return *this;
	}
	int GetData(char *&pData);   // Set pData to point to
								 //   the record's contents
	int GetRid(RID &rid);       // Get the record id
	int SetRecord(RID &new_rid, char *new_pdata, int new_size);
private:
	RID rid;
	char *data;
	int size;
};

class RM_FileHandle;
class RM_Manager {
	friend class RM_FileHandle;
public:
	RM_Manager(BFManager &new_bfm);
	~RM_Manager() {}
	int CreateFile(const char *fileName, int recordSize); // Create a new file, each with recordSize
	int RemoveFile(const char *fileName);
	int OpenFile(const char *fileName, RM_FileHandle &fileHandle);
	int CloseFile(RM_FileHandle &fileHandle);
private:
	BFManager &bfm;   // a block file manager to help with file and block operation
};  // important to use a reference, otherwise it will cause problems when destructed

class RM_FileHandle {   // rm filehandle can manipulate records
	friend class RM_Manager;
	friend class RM_FileScan;
public:
	RM_FileHandle();
	~RM_FileHandle();
	int GetRec(RID &rid, RM_Record &rec);  // get a record to rec according to rid

	int InsertRec(char *pData, RID &rid);      // insert a new record, return a rid

	int DeleteRec(RID &rid);                   // delete a record accorind to rid
	int UpdateRec(RM_Record &rec);            // update a record
	//int ForceBlocks(int blockNum);   // write dirty block
	int GetNextRec(int blockNum, int slotNum, BF_BlockHandle &bh, RM_Record &rec, bool nextMode);  // get next record, used in the filescan
private:
	bool open;
	RM_FileHeader header;
	BF_FileHandle bfh;
	bool hdr_dirty;
	inline int getBitmapAndBlockHdr(BF_BlockHandle bh, char *&bitmap, RM_BlockHeader *&blockHdr);
};
enum AttrType
{
	INT_, FLOAT_, STRING_
};
enum CompOp {
	EQ, LT, GT, LE, GE, NE, GTLT, GELT, GTLE, GELE, NO
	// equal, less than, great than, less than or equal ... no comparsion
};
class RM_FileScan {
public:
	RM_FileScan();                                // Constructor
	~RM_FileScan();                                // Destructor
	int OpenScan(const RM_FileHandle &fileHandle,  // Initialize file scan
		AttrType      attrType,
		int           attrLength,
		int           attrOffset,
		CompOp        compOp,
		void          *value,
		void		  *value2
	);
	int GetNextRec(RM_Record &rec);                  // Get next matching record
	int CloseScan();                                // Terminate file scan
private:
	bool openScan;
	RM_FileHandle *fileHandle;
	bool(*comparator) (void *, void *, AttrType, int, void *);
	int attrOffset;
	int attrLength;
	void *value;
	void *value2;  // for range check
	AttrType attrType;
	CompOp compOp;
	bool scanEnded;
	int scanBlock;
	int scanSlot;
	BF_BlockHandle currentBh;   // block handle of the current block
	int numRecOnBlock;
	int numSeenOnBlock;
	bool useNextBlock;
	bool hasBlockPinned;
	bool initializedValue;
};


bool equal(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3 = NULL);
bool less_than(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3 = NULL);
bool greater_than(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3 = NULL);
bool less_or_eq(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3 = NULL);
bool greater_or_eq(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3 = NULL);
bool gt_and_lt(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3);
bool gt_and_le(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3);
bool ge_and_lt(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3);
bool ge_and_le(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3);
bool not_equal(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3=NULL);


