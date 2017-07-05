#include "BF_BufferManager.h"
#include <cstdio>
#include <iostream>
#include <map>
#include "BF_internal.h"
const int INVALID_SLOT = -1;
using namespace std;

map<string,FILENO> name_fno;
map<FILENO,string> fno_name;

BF_BufferManager::BF_BufferManager(int blockNum){
	numBlocks = blockNum;
	blockSize = BF_BLOCK_SIZE + sizeof(blockHeader);

	bufTable = new BlockDesc[numBlocks];
	for (int i = 0; i < numBlocks; i++) {
		if ((bufTable[i].pData = new char[blockSize]) == NULL) {
			// allocate space for each block data
			cerr << "Not enough memory for buffer\n";
			exit(1);
		}

		memset((void *)bufTable[i].pData, 0, blockSize); // initialize data to zero

		bufTable[i].prev = i - 1;
		bufTable[i].next = i + 1;
	}
	bufTable[0].prev = bufTable[numBlocks - 1].next = INVALID_SLOT; // reset  boundary case
	free = 0;  // every block is in the free list
	first = last = INVALID_SLOT;
}

BF_BufferManager::~BF_BufferManager() {
	for (int i = 0; i < numBlocks; i++)
	{
		if (bufTable[i].dirty)//output all dirty blocks
			WriteBlock(bufTable[i].fno, bufTable[i].blockNum, bufTable[i].pData);
		delete[] bufTable[i].pData;   // first delete each data
	}

	delete[] bufTable;  // finally delete the whole bufTable
}

int BF_BufferManager::GetBlock(FILENO fno, int blockNum, char *&pBuffer) {
	int slot;
	Entry entry;
	entry.first = fno;
	entry.second = blockNum;
	unordered_map<Entry, int, entry_hash>::iterator iter;
	iter = hashTable.find(entry);
	if (iter == hashTable.end()) { // if the block is not in buffer
		if (allocate(slot)) // allocate a new block
			return -1;
		// read the block, insert it to the hash table, initialize the block
		//if (ReadBlock(file, blockNum, bufTable[slot].pData) ||
		//	hashTable.insert(make_pair<Entry, int>(entry, slot)) ||
		//	InitialBlockDesc(file, blockNum, slot)) {
		//		unlink(slot);
		//		insertFree(slot);  // if error occurs, put the slot back to the free list
		//		return -1;
		//}
		ReadBlock(fno, blockNum, bufTable[slot].pData);
		hashTable.insert({ entry, slot });
		InitialBlockDesc(fno, blockNum, slot);
	}
	else {  // find the block
		slot = (*iter).second;
		if (unlink(slot) || linkHead(slot))  // mark the block as the MRU one
			return -1;
		//bufTable[slot].pinCount++;  // increment pin count
	}
	pBuffer = bufTable[slot].pData;
	return 0;
}

//allocate a block for the file fno and block number blockNum
int BF_BufferManager::AllocateBlock(FILENO fno, int blockNum, char *&pBuffer) {
	int slot;
	Entry entry;
	entry.first = fno;
	entry.second = blockNum;
	unordered_map<Entry, int, entry_hash>::iterator iter;
	iter = hashTable.find(entry);
	if (iter != hashTable.end()) // if the block is already in the buffer, error
		return -1;
	if (allocate(slot))
		return -1;
	// insert the block to the hash table and initilize it
	/*if (hashtable.insert(file, blocknum, slot) ||
		initialblockdesc(file, blocknum, slot)) {
		unlink(slot);
		insertfree(slot);
		return -1;
	}*/
	hashTable.insert({ entry, slot });
	InitialBlockDesc(fno, blockNum, slot);
	pBuffer = bufTable[slot].pData;
	return 0;
}

int BF_BufferManager::MarkDirty(FILENO fno, int blockNum) {
	int slot;
	Entry entry;
	entry.first = fno;
	entry.second = blockNum;
	unordered_map<Entry, int, entry_hash>::iterator iter;
	iter = hashTable.find(entry);
	// the block should be in the buffer and pinned
	if (iter == hashTable.end())
		return -1;
	slot = (*iter).second;
	//if (!bufTable[slot].pinCount)
	//	return -1;
	bufTable[slot].dirty = true;
	if (unlink(slot) || linkHead(slot)) // mark the block the MRU one
		return -1;
	return 0;
}

int BF_BufferManager::UnpinBlock(FILENO fno, int blockNum) {
	int slot;
	Entry entry;
	entry.first = fno;
	entry.second = blockNum;
	unordered_map<Entry, int, entry_hash>::iterator iter;
	iter = hashTable.find(entry);
	// the block should be in the buffer and pinned
	if (iter == hashTable.end())
		return -1;
	slot = (*iter).second;
	if (!bufTable[slot].pinCount)
		return -1;
	if (bufTable[slot].pinCount == 0) return -1;
	if (--(bufTable[slot].pinCount) == 0) { // decrease the pin count
		if (unlink(slot) || linkHead(slot)) // if the block becomes unpinned
			return -1;						// mark the block the MRU one
	}
	return 0;
}

int BF_BufferManager::PinBlock(FILENO fno, int blockNum)
{
	int slot;
	Entry entry;
	entry.first = fno;
	entry.second = blockNum;
	unordered_map<Entry, int, entry_hash>::iterator iter;
	iter = hashTable.find(entry);
	// the block should be in the buffer and pinned
	if (iter == hashTable.end())
		return -1;
	slot = (*iter).second;
	bufTable[slot].pinCount++;
	return 0;
}

int BF_BufferManager::FlushBlocks(FILENO fno) {
	int slot, next;
	slot = first;
	int flag = 0;

	while (slot != -1) {
		next = bufTable[slot].next;

		//if is belong to the file
		if (bufTable[slot].fno == fno) {

			// write back dirty block
			if (bufTable[slot].dirty) { 
				if (WriteBlock(fno, bufTable[slot].blockNum, bufTable[slot].pData))
					flag = -1;
				else
					bufTable[slot].dirty = false;
			}

			if (!bufTable[slot].pinCount && !bufTable[slot].dirty) 
			{ 
				// delete the block from the hash table and add the slot to the free list
				/*if (hashTable[slot].Delete(file, bufTable[slot].blockNum) ||
					unlink(slot) || insertFree(slot)
					)
					return -1;*/

				hashTable.erase(Entry(fno, bufTable[slot].blockNum));
				unlink(slot);
				insertFree(slot);
			}
		}
		slot = next;
	}
	return flag;
}

int BF_BufferManager::ForceBlocks(FILENO fno)
{
	int slot, next;
	slot = first;
	int flag = 0;

	while (slot != -1) {
		next = bufTable[slot].next;

		//if is belong to the file
		if (bufTable[slot].fno == fno) {

			// write back dirty block
			if (bufTable[slot].dirty) { 
				if (WriteBlock(fno, bufTable[slot].blockNum, bufTable[slot].pData))
					flag = -1;
				else
					bufTable[slot].dirty = false;
			}
		}
		slot = next;
	}
	return flag;
}

void BF_BufferManager::dump()
{
	cout << "free list: " << endl;
	for (int i = free; i != INVALID_SLOT; i = bufTable[i].next) {
		cout << "i= " << i << " blockNum= " << bufTable[i].blockNum << " prev= " << bufTable[i].prev <<
			" next= " << bufTable[i].next << " -> ";
	}
	cout << "\n\n";
	cout << "used list " << endl;
	for (int i = first; i != INVALID_SLOT; i = bufTable[i].next) {
		cout << "i= " << i << " blockNum= " << bufTable[i].blockNum << " prev= " << bufTable[i].prev <<
			" next= " << bufTable[i].next << " -> ";
	}
	cout << "first,last " << first << " " << last<<endl;
	cout << "\n\n";
}

int BF_BufferManager::insertFree(int slot) { // insert a slot at the start of free list
	bufTable[slot].next = free;
	free = slot;
	return 0;
}

int BF_BufferManager::linkHead(int slot) { // insert a slot at the start of used list
	/*int old_prev = bufTable[slot].prev, old_next = bufTable[slot].next;
	if (old_next != INVALID_SLOT)
		bufTable[old_next].prev = old_prev;
	if (old_prev != INVALID_SLOT)
		bufTable[old_prev].next = old_next;*/
	bufTable[slot].next = first;
	bufTable[slot].prev = INVALID_SLOT;
	if (first != INVALID_SLOT)  // the used list is not empty
		bufTable[first].prev = slot;
	first = slot;
	if (last == INVALID_SLOT)   // the used list is empty
		last = first;
	return 0;
}

int BF_BufferManager::unlink(int slot) { // unlink the slot from the used list
	if (slot == first)
		first = bufTable[slot].next;
	if (slot == last)
		last = bufTable[slot].prev;
	int old_prev = bufTable[slot].prev, old_next = bufTable[slot].next;
	if (old_next != INVALID_SLOT)
		bufTable[old_next].prev = old_prev;
	if (old_prev != INVALID_SLOT)
		bufTable[old_prev].next = old_next;
	bufTable[slot].prev = bufTable[slot].next = INVALID_SLOT;
	return 0;
}

// allocate a slot, use one in the free list in possible
// otherwise replace one in the used list
int BF_BufferManager::allocate(int &slot) { 
								   
	if (free != INVALID_SLOT) { // free list is avaliable
	  slot = free;
	  free = bufTable[slot].next;
   }
   else {
	  // Choose the least-recently used page that is unpinned
	  for (slot = last; slot != INVALID_SLOT; slot = bufTable[slot].prev) {
		 if (bufTable[slot].pinCount == 0)
			break;
	  }

	  // Return error if all buffers were pinned
	  if (slot == INVALID_SLOT)
		 return -1;

	  // Write out the page if it is dirty
	  if (bufTable[slot].dirty) {
		 if (WriteBlock(bufTable[slot].fno, bufTable[slot].blockNum, bufTable[slot].pData))
			return -1;

		 bufTable[slot].dirty = false;
	  }

	  Entry entry;
	  entry.first = bufTable[slot].fno;
	  entry.second = bufTable[slot].blockNum;
	  // Remove page from the hash table and slot from the used buffer list
	  /*if (hashTable.Delete(bufTable[slot].file, bufTable[slot].blockNum) ||
			(unlink(slot)))
		 return -1;*/
	  hashTable.erase(entry);
	  unlink(slot);
   }

   // Link slot at the head of the used list
   if (linkHead(slot))
	  return -1;

   // Return ok
   return 0;
}

int BF_BufferManager::ReadBlock(FILENO fno, int blockNum, char *dest) {

	FILE* file;
	file = fopen(fno_name[fno].data(),"rb+");
	if (!file) return -1;

	int offset = blockNum * blockSize + BF_BLOCK_SIZE + sizeof(blockHeader);
	if (fseek(file, offset, SEEK_SET))
	{
		fclose(file);
		return -1;
	}
	int numByte = fread(dest, sizeof(char), blockSize, file);
	if (numByte != blockSize)
	{
		fclose(file);
		return -1;
	}
	fclose(file);
	return 0;
}

//only write to disk, not free the space in buffer
int BF_BufferManager::WriteBlock(FILENO fno, int blockNum, char *src) {
	FILE* file;
	file = fopen(fno_name[fno].data(),"rb+");
	if (!file) return -1;

	int offset = blockNum * blockSize + BF_BLOCK_SIZE + sizeof(blockHeader);
	if (fseek(file, offset, SEEK_SET))
	{
		fclose(file);
		return -1;
	}
	int numByte = fwrite(src, sizeof(char), blockSize, file);
	if (numByte != blockSize)
	{
		fclose(file);
		return -1;
	}
	fclose(file);
	return 0;
}

int BF_BufferManager::InitialBlockDesc(FILENO fno, int blockNum, int slot) {
	bufTable[slot].fno = fno;
	bufTable[slot].blockNum = blockNum;
	bufTable[slot].dirty = false;
	bufTable[slot].pinCount = 1;
	return 0;
}
