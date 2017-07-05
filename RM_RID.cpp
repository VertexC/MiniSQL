#include "RM_RID.h"
RID::RID() {
	blockNum = -1;
	slotNum = -1;
}
RID::RID(int blockNum, int slotNum) {
	this->blockNum = blockNum;
	this->slotNum = slotNum;
}
int RID::GetBlockNum(int &block) {
	block = blockNum;
	return 0;
}
int RID::GetSlotNum(int &slot) {
	slot = slotNum;
	return 0;
}