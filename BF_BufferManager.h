#pragma once
#include <unordered_map>
#include "BF_internal.h"
using namespace std;

struct BlockDesc {
	char *pData; // ptr to data
	int next; // next in the linked list
	int prev; // prev in the linked list
	bool dirty;
	int pinCount;
	int blockNum;
	FILENO fno;
};

typedef pair<FILENO, int> Entry;
struct entry_hash{
	size_t operator()(const	Entry &name) const
	{
		return hash<int>()(name.first) ^ hash<int>()(name.second);
	}
};

class BF_BufferManager {
public:
	BF_BufferManager(int numBlock);
	~BF_BufferManager();
	int GetBlock(FILENO fno, int blockNum, char *&ppBuffer);
	int AllocateBlock(FILENO fno, int blockNum, char *&ppBuffer);
	int MarkDirty(FILENO fno, int blockNum);
	int UnpinBlock(FILENO fno, int blockNum);
	int PinBlock(FILENO fno, int blockNum);
	void dump();  // test, dump the bufTable

	//write all dirty blocks to disk for the given file and
	//remove unpin blocks for the given file
	int FlushBlocks(FILENO fno);
	//write all dirty blocks to disk but don't remove them
	int ForceBlocks(FILENO fno);

private:
	int insertFree(int slot);
	int linkHead(int slot);
	int unlink(int slot);
	int allocate(int &slot);

	int ReadBlock(FILENO fno, int blockNum, char *dest);
	int WriteBlock(FILENO fno, int blockNum, char *src);
	int InitialBlockDesc(FILENO fno, int blockNum, int slot);

	BlockDesc *bufTable;
	unordered_map<Entry, int, entry_hash> hashTable;
	int numBlocks;
	int blockSize;
	int first;  // MRU block slot
	int last;   // LRU block slot
	int free;  // head of the free list
};

extern map<string,FILENO> name_fno;
extern map<FILENO,string> fno_name;
