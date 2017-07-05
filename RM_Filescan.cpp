#include "RM.h"
#include <iostream>
using namespace std;
RM_FileScan::RM_FileScan() {
	openScan = false; // initially a filescan is not valid
	value = value2 = NULL;
	initializedValue = false;
	hasBlockPinned = false;
	scanEnded = true;
}
RM_FileScan::~RM_FileScan() {
	if (scanEnded == false && hasBlockPinned == true && openScan == true) {
		//fileHandle->bfh.UnpinBlock(scanBlock);
	}
	if (initializedValue == true) { // free any memory that are not freed
		free(value);
		initializedValue = false;
	}
}

bool equal(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3) {
	switch (attrtype) {
	case INT_: return (*(int *)value1 == *(int *)value2); break;
	case FLOAT_: return (*(float *)value1 == *(float*)value2); break;
	case STRING_:
		return (strncmp((char *)value1, (char *)value2, attrLength) == 0); break;
	default: cout << "wrong type" << endl; return true; break;
	}
}
bool less_than(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3) {
	switch (attrtype) {
	case INT_: return (*(int *)value1 < *(int *)value2); break;
	case FLOAT_: return (*(float *)value1 < *(float*)value2); break;
	case STRING_:
		return (strncmp((char *)value1, (char *)value2, attrLength) < 0); break;
	default: cout << "wrong type" << endl; return true; break;
	}
}
bool greater_than(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3) {
	switch (attrtype) {
	case INT_: return (*(int *)value1 > *(int *)value2); break;
	case FLOAT_: return (*(float *)value1 > *(float*)value2); break;
	case STRING_:
		return (strncmp((char *)value1, (char *)value2, attrLength) > 0); break;
	default: cout << "wrong type" << endl; return true; break;
	}
}
bool less_or_eq(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3) {
	return !greater_than(value1, value2, attrtype, attrLength);
}
bool greater_or_eq(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3) {
	return !less_than(value1, value2, attrtype, attrLength);
}
bool gt_and_lt(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3){
	return greater_than(value1, value2, attrtype, attrLength) &&
		less_than(value1, value3, attrtype, attrLength);
}
bool gt_and_le(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3){
	return greater_than(value1, value2, attrtype, attrLength) &&
		less_or_eq(value1, value3, attrtype, attrLength);
}
bool ge_and_lt(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3) {
	return greater_or_eq(value1, value2, attrtype, attrLength) &&
		less_than(value1, value3, attrtype, attrLength);
}
bool ge_and_le(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3) {
	return greater_or_eq(value1, value2, attrtype, attrLength) &&
		less_or_eq(value1, value3, attrtype, attrLength);
}
bool not_equal(void * value1, void * value2, AttrType attrtype, int attrLength, void *value3) {
	return !equal(value1, value2, attrtype, attrLength);
}

int RM_FileScan::OpenScan(const RM_FileHandle &new_fileHandle,
	AttrType   attrType,
	int        attrLength,
	int        attrOffset,
	CompOp     compOp,
	void       *value,
	void	   *value2)
{
	if (openScan)  // the file should not be associated with other scan
		return -1;
	//if (fileHandle.isValidFileHeader()) // check whether file header is valid
	fileHandle = const_cast<RM_FileHandle*>(&new_fileHandle);
	//else
		//return -1;
	this->value = this->value2 = NULL;
	this->compOp = compOp;
	switch (compOp) {
		case EQ: comparator = &equal; break;
		case LT: comparator = &less_than; break;
		case GT: comparator = &greater_than; break;
		case LE: comparator = &less_or_eq; break;
		case GE: comparator = &greater_or_eq; break;
		case NE: comparator = &not_equal; break;
		case GTLT: comparator = &gt_and_lt; break;
		case GTLE: comparator = &gt_and_le; break;
		case GELT: comparator = &ge_and_lt; break;
		case GELE: comparator = &ge_and_le; break;
		case NO: comparator = NULL; break;
		default: return -1; // (RM_INVALIDSCAN);
	}

	int recSize = fileHandle->header.recordSize;
	// If there is a comparison, update the comparison parameters.
	if (this->compOp != NO) {
		// Check that the attribute offset and sizes are compatible with given
		// FileHandle
		if ((attrOffset + attrLength) > recSize || attrOffset < 0 || attrOffset > MAXSTRINGLEN)
			return -1;// (RM_INVALIDSCAN);
		this->attrOffset = attrOffset;
		this->attrLength = attrLength;

		// Allocate the appropraite memory to store the value being compared
		/*if (attrType == FLOAT || attrType == INT) {
			if (attrLength != 4)
				return -1;//(RM_INVALIDSCAN);
			this->value = (void *)malloc(4);
			memcpy(this->value, value, 4);
			if (value2) {  // maybe NULL
				this->value2 = (void *)malloc(4);
				memcpy(this->value2, value2, 4);
			}
			initializedValue = true;
		}
		else if (attrType == STRING) {
			this->value = (void *)malloc(attrLength);
			memcpy(this->value, value, attrLength);
			if (value2) {  // maybe NULL
				this->value2 = (void *)malloc(attrLength);
				memcpy(this->value2, value2, attrLength);
			}
			initializedValue = true;
		}
		else {
			return -1; // (RM_INVALIDSCAN);
		}*/
		this->value = (void *)malloc(attrLength);
		memcpy(this->value, value, attrLength);
		if (value2) {  // maybe NULL
			this->value2 = (void *)malloc(attrLength);
			memcpy(this->value2, value2, attrLength);
		}
		initializedValue = true;
		this->attrType = attrType;
	}

	// open the scan
	openScan = true;
	scanEnded = false;

	// set up scan parameters:
	numRecOnBlock = 0;
	numSeenOnBlock = 0;
	useNextBlock = true;
	scanBlock = 0;
	scanSlot = -1;
	numSeenOnBlock = 0;
	hasBlockPinned = false;
	return (0);
}
int RM_FileScan::GetNextRec(RM_Record &rec) {
	// If the scan has ended, or is not valid, return immediately
	if (scanEnded == true)
		return -1; // (RM_EOF);
	if (openScan == false)
		return -1; // (RM_INVALIDSCAN);
	hasBlockPinned = true;

	int rc;
	while (true) {
		// Retrieve next record
		RM_Record temp;
		if (fileHandle->GetNextRec(scanBlock, scanSlot, currentBh, temp, useNextBlock) == -1) {
			hasBlockPinned = false;
			scanEnded = true;
			return -1;
		}
		hasBlockPinned = true;
		// If we retrieved a record on the next Block, reset numRecOnBlock to
		// reflect the number of records seen on this new current Block
		if (useNextBlock) {
			char *pdata;
			currentBh.getData(pdata);
			numRecOnBlock = (reinterpret_cast<RM_BlockHeader *>(pdata))->numRecords;
			useNextBlock = false;
			numSeenOnBlock = 0;
			if (numRecOnBlock == 1)
				currentBh.getBlockNum(scanBlock);
		}
		numSeenOnBlock++; // update # of records seen on this Block

						 // If we've seen all the record on this Block, then next time, we
						 // need to look on the next Block, not this Block, so unpin the Block
						 // and set the indicator (useNextBlock)
		if (numRecOnBlock == numSeenOnBlock) {
			useNextBlock = true;
			//printf("unpin Block in filescan\n");
			//fileHandle->bfh.UnpinBlock(scanBlock);
			hasBlockPinned = false;
		}

		// Retrieves the RID of the scan to update the progress of the scan
		RID rid;
		temp.GetRid(rid);
		rid.GetBlockNum(scanBlock);
		rid.GetSlotNum(scanSlot);

		// Check to see if it satisfies the scan comparison, and if it does,
		// exit the function, returning the record.
		char *pData;
		if ((rc = temp.GetData(pData))) {
			return (rc);
		}
		if (compOp != NO) {
			bool satisfies = (*comparator)(pData + attrOffset, this->value, attrType, attrLength, this->value2);
			if (satisfies) {
				rec = temp;
				break;
			}
		}
		else {
			rec = temp; // if no comparison, just return the record
			break;
		}
	}
	return 0;
}
int RM_FileScan::CloseScan() {   // free the space to hold the values to be compeared and unpin the block
	int rc;
	if (openScan == false) {
		return -1;  // (RM_INVALIDSCAN);
	}
	/*if (hasBlockPinned == true) {
		//printf("unpinning Block\n");
		if ((rc = fileHandle->bfh.UnpinBlock(scanBlock)))
			return (rc);
	}*/
	if (initializedValue == true) {
		free(this->value);
		initializedValue = false;
	}
	openScan = false;
	return (0);
}

