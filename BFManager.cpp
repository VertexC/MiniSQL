#include "BFManager.h"
#include <fstream>
#include "BF_internal.h"

using namespace std;

BFManager bfm;
int BFManager::fno_now = 0;

BFManager::BFManager() {
	buffer_pool = new BF_BufferManager(BF_BUFFER_SIZE);
}

BFManager::~BFManager() {
	delete buffer_pool;
}

int BFManager::createFile(const char *fileName) {
	FILE *fio;
	if (fio = fopen(fileName, "r")) { // this file already exists, error
		fclose(fio);
		return -1;
	}
	fio = fopen(fileName, "wb");  // new file will be created if not exists
	if (!fio) { //  have error opening the file
		return -1;
	}
	// initialize the file header
	char file_header[BF_BLOCK_SIZE + sizeof(BF_FileHdr)];
	memset(file_header, 0, BF_BLOCK_SIZE + sizeof(BF_FileHdr));

	BF_FileHdr *header = (BF_FileHdr *)file_header;
	header->firstFree = -1;
	header->numBlocks = 0;

	if (fwrite(header, sizeof(char), BF_BLOCK_SIZE + sizeof(BF_FileHdr), fio) != BF_BLOCK_SIZE + sizeof(BF_FileHdr))
		return -1;   // error if wrong # of bytes are written
	if (fclose(fio) == EOF)
		return -1;  // have error closing the file
	return 0;
}

int BFManager::removeFile(const char *fileName) {
	if (remove(fileName))
		return -1;  // error removing the file(ret = -1)

	map<string,FILENO>::iterator it;
	it = name_fno.find(fileName);
	if (it != name_fno.end())
	{
		//open before
		fno_name.erase(fno_name.find(it->second));
		name_fno.erase(it);
	}
	return 0;
}

int BFManager::openFile(const char *fileName, BF_FileHandle &fileHandle) {
	FILE *file;
	FILENO fno;
	map<string,FILENO>::iterator it;
	map<FILENO,string>::iterator it2;

	if (fileHandle.fileOpen) // filehandle already have a open file associated with it
		return -1;

	if (!(file= fopen(fileName, "rb+")))  // have to be appendix, if it is write then the contents are cleaned
		return -1;  // error opening the file

	//get fno
	it = name_fno.find(fileName);
	if (it != name_fno.end())
	{
		//open before, may exist in buffer
		fno = it->second;
	}
	else
	{
		fno = ++fno_now;
		//map filename to fno
		name_fno.insert(make_pair(string(fileName), fno));
		fno_name.insert(make_pair(fno, string(fileName)));
	}
	fileHandle.fno = fno;

	//fseek(fileHandle.file, 0, SEEK_SET);  // move the file handle to the start
	int c= fread((char *)&fileHandle.header, sizeof(char), sizeof(fileHeader), file);
	//if (fread(&fileHandle.header, sizeof(char), sizeof(fileHeader), fileHandle.file) != sizeof(fileHeader))
	//	return -1;  // error reading the header
	fileHandle.fileOpen = true;
	fileHandle.hdrDirtyFlag = false;
	fileHandle.bufMgr = buffer_pool;
	fclose(file);
	return 0;
}

int BFManager::closeFile(BF_FileHandle &fileHandle) {
	if (!fileHandle.fileOpen) // the file handle should be opened
		return -1;
	int ret;
	//FILE* file = filetable[fileHandle.fno];
	if (ret = fileHandle.FlushBlocks()) // flush all blocks(and remove them from buffer)
		return ret;  // ret != 0 means error
	//if (fclose(file) == EOF)
	//	return -1;  // error closing the file og filehandle
	fileHandle.fileOpen = false;
	fileHandle.bufMgr = NULL;

	return 0;
}
