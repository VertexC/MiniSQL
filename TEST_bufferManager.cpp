#define _CRT_SECURE_NO_WARNINGS
#include "BF_BufferManager.h"
#include <cstdio>
#include <iostream>
using namespace std;

int main() {
	FILE *file;
	char **ppbuf = new char*;
	file = fopen("buffer.txt", "w+");
	BF_BufferManager bufMgr(3);
	bufMgr.dump();
	for (int i = 0; i < 12; i++) {
		cout << "time: " << i << endl;
		bufMgr.AllocateBlock(file, i, ppbuf);
		bufMgr.dump();
		if (i != 1)
		bufMgr.UnpinBlock(file, i);   // a block has to be unpinned after use
		// otherwise it remains in the buffer
		//cout << "unpin\n";
		//bufMgr.dump();
		cout << "\n======\n";

	}
	return 0;
}
