#include "RM.h"
RM_Record::RM_Record() {
	size = -1;
	data = NULL;
}
RM_Record::~RM_Record() {
	if (data != NULL)
		delete[]data;
}
int RM_Record::GetData(char *&pdata) {
	if (data == NULL || size == -1)
		return -1;
	pdata = data;
	return 0;
}
int RM_Record::GetRid(RID &Rid) {
	Rid = rid;
	return 0;
}
int RM_Record::SetRecord(RID &new_rid, char *new_data, int new_size) {
	rid = new_rid;
	size = new_size;
	if (!data)
		data = new char[size];
	memcpy(data, new_data, new_size);
	return 0;
}