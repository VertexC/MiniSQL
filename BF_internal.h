#pragma once
/* next free is
1.next block number
2. -1,the end of free list
3. -2 not in the free list
*/
#include <cstring>
#include <map>
struct blockHeader {
	int nextFree;
};
const int BF_BLOCK_SIZE = 4096 - sizeof(blockHeader);  // 8 for test
const int BF_BUFFER_SIZE = 4096;//num of blocks in buffer

typedef int FILENO;
extern std::map<FILENO, FILE*> filetable;




