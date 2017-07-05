#pragma once

class RID {
public:
	RID();
	RID(int pageNum, int slotNum);
	int GetBlockNum(int &blockNum);
	int GetSlotNum(int &slotNum);
private:
	int blockNum;
	int slotNum;
};
