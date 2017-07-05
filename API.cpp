#include "API.H"


API::API(){
	index_m = new Index_Manager();
	record_m = new RM_Manager(blockf_manager);
}

API::~API(){
	delete index_m;
	delete record_m;
}

void API::package_resolve(Package * package){
	if(package->type == Package::CREATEI_PACK){
		CreateIPack * ctp = dynamic_cast<CreateIPack *>(package);
		index_create(ctp);
	}
	else if(package->type == Package::DROPI_PACK){
		DropIPack * ctp = dynamic_cast<DropIPack *>(package);
		index_drop(ctp);
	}
	else if(package->type == Package::CREATET_PACK){
		CreateTPack * ctp = dynamic_cast<CreateTPack *>(package);
		table_create(ctp);
	}
	else if(package->type == Package::DROPT_PACK){
		DropTPack * ctp = dynamic_cast<DropTPack *>(package);
		table_drop(ctp);
	}
	else if(package->type == Package::SELECT_PACK){
		SelectPack * ctp = dynamic_cast<SelectPack *>(package);
		selection_query(ctp);
	}
	else if(package->type == Package::INSERT_PACK){
		InsertPack * ctp = dynamic_cast<InsertPack *>(package);
		record_insert(ctp);
	}
	else if(package->type == Package::DELETE_PACK){
		DeletePack * ctp = dynamic_cast<DeletePack *>(package);
		record_delete(ctp);		
	}
		
}

void API::type_trans(CreateTPack::Attri & attri, int & type){
	if(attri.type == Package::INT_)
		type = -1;
	else if(attri.type == Package::FLOAT_)
		type = 0;
	else 
		type = attri.size;
}

void API::table_create(CreateTPack * package){
	//done
	//buid up table in catalog
	CatlogManager::createTable(package);
	//build up index for primary key in indexmanager
	char * index_file = NULL;
	int type;
	CreateTPack::Attri attri;
	CatlogManager::getIndex(package->table, package->PK, index_file);
	CatlogManager::getAttribute(package->table, package->PK, attri);
	type_trans(attri, type);
	index_m->create_Index(index_file, type);
	
	char *file_record; 
	int sum = 0;
	CatlogManager::getTable(package->table, file_record);
	
	vector<CreateTPack::Attri> attributes;
	CatlogManager::getAttribute(package->table, attributes);
	vector<CreateTPack::Attri>::iterator it_attributes;
	for (it_attributes = attributes.begin(); it_attributes != attributes.end(); it_attributes++)
	{
		sum += it_attributes->size;
	}
	record_m->CreateFile(file_record, sum); 
	cout << "MiniSQL::Table <" << package->table << "> Create." <<endl;
	delete[] index_file;
	delete[] file_record;
}

void API::table_drop(DropTPack * package){
	//done
	vector<CreateTPack::Attri> attributes;
	CatlogManager::getAttribute(package->table, attributes);
	//drop index in the index manager
	vector<CreateTPack::Attri>::iterator it_attri;
	for (it_attri = attributes.begin(); it_attri != attributes.end(); ++it_attri)
	{
		char * file_index = NULL;
		if(CatlogManager::getIndex(package->table, it_attri->name, file_index)){
			//have index to drop
			index_m->drop_Index(file_index);
			delete[] file_index;
		}
	}
	//drop the record table
	char * file_record;
	CatlogManager::getTable(package->table, file_record);
	record_m->RemoveFile(file_record);
	//drop table in catalog
	CatlogManager::dropTable(package->table);
	cout << "MiniSQL::Table <" << package->table << "> Drop." << endl;
    delete file_record;
}


void API::record_insert(InsertPack * package){
	//done
	//add record in the recordmanager
	bool success = 1;
	char * record_data;
	vector<CreateTPack::Attri> attributes;
	vector<CreateTPack::Attri>::iterator it_attri;
	vector<InsertPack::Value>::iterator it_value;
	CatlogManager::getAttribute(package->table, attributes);
	//get the size
	int record_size = 0;
	for (it_attri = attributes.begin(); it_attri != attributes.end(); it_attri++)
	{
		record_size += it_attri->size;
	}
	record_data = (char *)malloc(sizeof(char) * record_size);
	//copy the data
	int pivot = 0;
	it_attri = attributes.begin();
	for (it_value = package->value.begin(); it_value != package->value.end(); it_value++)
	{
		if(it_value->type == Package::INT_){
			memcpy(record_data + pivot, (int *)&it_value->value.i, it_value->size);
		}
		else if(it_value->type == Package::FLOAT_){
			memcpy(record_data + pivot, (float *)&it_value->value.f, it_value->size);
		}
		else{
			memcpy(record_data + pivot, it_value->value.c, it_value->size);
		}
		pivot+=it_attri->size;
		it_attri++;
	}
	//get file_record
	RM_FileHandle fhd;
	char * file_record = NULL;
	CatlogManager::getTable(package->table, file_record);
	record_m->OpenFile(file_record, fhd);
	RID rid; 
	fhd.InsertRec(record_data, rid);
	
	//add index attributes in recordmanager
	it_value = package->value.begin(); 
	for (it_attri = attributes.begin(); it_attri < attributes.end(); ++it_attri)
	{
		char * file_index = NULL;
		if (CatlogManager::getIndex(package->table, it_attri->name, file_index))
		{
			//get key_value
			string key_value;
			stringstream stream;
			if(it_attri->type == Package::INT_){
				stream << it_value->value.i;
				stream >> key_value;
			}
			else if (it_attri->type == Package::FLOAT_)
			{
				stream << it_value->value.f;
				stream >> key_value;
			}
			else 
				key_value = it_value->value.c;
			//insert to the indexmanager
			int type;
			type_trans(*it_attri, type);
			if(index_m->insert_Index(file_index, key_value, rid, type) == false){
				//the value already exist;
				fhd.DeleteRec(rid);
				success = 0;
			}	
			//delete[] file_index;
		}
		it_value ++;
	}
	record_m->CloseFile(fhd);
	if(success) cout << "MiniSQL::Insert record Done." << endl;
	else cout << "MiniSQL::Duplicate value of primary key or unique attributes." << endl;
	delete[] file_record;
}

void API::record_delete(DeletePack * package){
	//done
	 char * file_record = NULL;
	vector<CreateTPack::Attri>attributes;
	CatlogManager::getAttribute(package->table, attributes);
	
	//get the offset of the atritbutes
	vector<CreateTPack::Attri>::iterator it_attri;
	CatlogManager::getTable(package->table, file_record);
	RM_FileHandle fhd;
	record_m->OpenFile(file_record, fhd);
	RM_FileScan scan;
	RM_Record rec;
	RID rid;
	int attrLength;
	int attrOffset;
	int compOp = 0;
	int value1;
	int value2;
	int attrType = 0;
	vector<RID> result_rid;
	vector<RID>::iterator it_result_rid;
	vector<RID>::iterator it_result_rid_temp;
	vector<DeletePack::Condition>::iterator it_condition;
	if (package->condition.size() == 0)//no condition
	{
		//delete in record manager
		compOp = 10;//no condition deletion
		scan.OpenScan(fhd, (AttrType)attrType, attrLength, attrOffset, (CompOp)compOp, &value1, &value2); 
		while (scan.GetNextRec(rec) != -1){
			rec.GetRid(rid);
			//delete the index in the index manager according to rid
			for(it_attri = attributes.begin(); it_attri != attributes.end(); ++it_attri)
			{
				char * file_index = NULL;
				if(CatlogManager::getIndex(package->table, it_attri->name, file_index)){
					int type;
					type_trans(*it_attri, type);
					index_m->delete_Index(file_index, rid, it_attri->type);
					delete[] file_index;
				}
			}
			result_rid.push_back(rid);
		}
	}
	else{
		//there is one or more condition
		for (it_condition = package->condition.begin(); it_condition != package->condition.end(); ++it_condition){
			//get the attrOffset
			attrOffset = 0;
			for (it_attri = attributes.begin(); it_attri < attributes.end(); ++it_attri){
				if (strcmp(it_condition->attri, it_attri->name) == 0){
					//attrOffset += it_attri->size;
					attrLength = it_attri->size;
					break;
				}
				else 
					attrOffset += it_attri->size;
			}
			if(it_condition == package->condition.begin()){
				//the first condtition push all the rid satisfied
				char * file_index = NULL;
				if(it_condition->type == INT_)
					scan.OpenScan(fhd, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.i), &value2);
				else if(it_condition->type == FLOAT_)
					scan.OpenScan(fhd, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.f), &value2);
				else
					scan.OpenScan(fhd, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.c), &value2);
	
				while (scan.GetNextRec(rec) != -1){
						rec.GetRid(rid);
						result_rid.push_back(rid);
				}	
			}	
			else{
				//not the fist condtion
				for(it_result_rid = result_rid.begin(); it_result_rid != result_rid.end();){
					it_result_rid_temp = it_result_rid;
					fhd.GetRec(* it_result_rid, rec);
					char * data;
					bool satisfied = 0;
					rec.GetData(data);
					if(it_condition->type == INT_)
						satisfied = satisfy(data, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.i), &value2);
					else if(it_condition->type == FLOAT_)
						satisfied = satisfy(data, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.f), &value2);
					else
						satisfied = satisfy(data, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.c), &value2);
					
					if(!satisfied)
						it_result_rid = result_rid.erase(it_result_rid_temp);
					else 
						it_result_rid++;
				}
				
			}		
		}
	}
	cout << "MiniSQL::Delete " << result_rid.size() << " row(s)." << endl; 
	for(it_result_rid = result_rid.begin(); it_result_rid != result_rid.end(); it_result_rid++){
		//delete the index in the index manager according to rid
		for(it_attri = attributes.begin(); it_attri != attributes.end(); ++it_attri){
			char * file_index = NULL;
			if(CatlogManager::getIndex(package->table, it_attri->name, file_index)){
				int type;
				type_trans(*it_attri, type);
				index_m->delete_Index(file_index, * it_result_rid, it_attri->type);
			    delete[] file_index;
			}
		}
		fhd.DeleteRec(*it_result_rid);
	}
	record_m->CloseFile(fhd);
	delete[] file_record;
}

bool API::satisfy(char * pData, AttrType attrType, int attrLength, int attrOffset, CompOp compOp, void * value1, void * value2){
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
	bool satisfied = (*comparator)(pData + attrOffset, value1, attrType, attrLength, value2);
	return satisfied;
} 

	

void API::index_create(CreateIPack * package){
	//done
	//buid up index in catalog
	CatlogManager::createIndex(package);
	//build up index in indexmanager
	 char * index_file = NULL;
	int type;
	CreateTPack::Attri attri;
	CatlogManager::getIndex(package->table, package->attri, index_file);
	CatlogManager::getAttribute(package->table, package->attri, attri);
	type_trans(attri, type);
	index_m->create_Index(index_file, type);
	cout << "MiniSQL::Index of <" << package->attri << "> Create." << endl; 
	delete[] index_file;
}

void API::index_drop(DropIPack * package){
	//done
	 char * file_name;
	CatlogManager::getIndex(package->index, file_name);
	index_m->drop_Index(file_name);
	//drop table in catalog
	CatlogManager::dropIndex(package->index);
	cout << "MiniSQL::Index <" << package->index << "> Drop." << endl; 
	delete[] file_name;
}

void API::selection_query(SelectPack * package){
	//undone onlu suppor leanear search now
	 char * file_record = NULL;
	vector<CreateTPack::Attri>attributes;
	CatlogManager::getAttribute(package->table, attributes);
	
	vector<CreateTPack::Attri>::iterator it_attri;
	CatlogManager::getTable(package->table, file_record);
	RM_FileHandle fhd;
	record_m->OpenFile(file_record, fhd);
	RM_FileScan scan;
	RM_Record rec;
	RID rid;
	int attrLength;
	int attrOffset;
	int compOp;
	int value1;
	int value2;
	vector<char *> result;
	vector<RID> result_rid;
	vector<char *>::iterator it_result; 
	vector<RID>::iterator it_result_rid;
	vector<RID>::iterator it_result_rid_temp;
	vector<SelectPack::Condition>::iterator it_condition;
	if (package->condition.size() == 0)//no condition
	{
		//select in record manager
		compOp = 10;//no condition search
		scan.OpenScan(fhd, AttrType(), attrLength, attrOffset, (CompOp)compOp, &value1, &value2); 
		while (scan.GetNextRec(rec) != -1){
			rec.GetRid(rid);
			result_rid.push_back(rid);
		}
	}
	else{
		//there is one or more condition
		for (it_condition = package->condition.begin(); it_condition != package->condition.end(); ++it_condition){
			//get the attrOffset
			attrOffset = 0;
			for (it_attri = attributes.begin(); it_attri != attributes.end(); ++it_attri){
				if (strcmp(it_condition->attri, it_attri->name) == 0){
					//attrOffset += it_attri->size;
					attrLength = it_attri->size;
					break;
				}
				else 
					attrOffset += it_attri->size;
			}
			if(it_condition == package->condition.begin()){
				//the first condtition push all the rid satisfied
				char * file_index = NULL;
				if(CatlogManager::getIndex(package->table, it_condition->attri, file_index) && it_condition->op != NE){
					if(it_condition->type == INT_)
						index_m->Index_search(file_index, &it_condition->value.i, (AttrType)it_condition->type, (CompOp)it_condition->op, result_rid);
					else if(it_condition->type == FLOAT_)
						index_m->Index_search(file_index, &it_condition->value.i, (AttrType)it_condition->type, (CompOp)it_condition->op, result_rid);
					else
						index_m->Index_search(file_index, &it_condition->value.i, (AttrType)it_condition->type, (CompOp)it_condition->op, result_rid);
				}
				else{
					if(it_condition->type == INT_)
						scan.OpenScan(fhd, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.i), &value2);
					else if(it_condition->type == FLOAT_)
						scan.OpenScan(fhd, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.f), &value2);
					else
						scan.OpenScan(fhd, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.c), &value2);		
					while (scan.GetNextRec(rec) != -1){
						rec.GetRid(rid);
						result_rid.push_back(rid);
					}	
				}	
			}	
			else{
				//not the fist condtion
				for(it_result_rid = result_rid.begin(); it_result_rid != result_rid.end();){
					it_result_rid_temp = it_result_rid;
					fhd.GetRec(* it_result_rid, rec);
					char * data;
					bool satisfied = 0;
					rec.GetData(data);
					if(it_condition->type == INT_)
						satisfied = satisfy(data, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.i), &value2);
					else if(it_condition->type == FLOAT_)
						satisfied = satisfy(data, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.f), &value2);
					else
						satisfied = satisfy(data, (AttrType)it_condition->type, attrLength, attrOffset, (CompOp)it_condition->op, &(it_condition->value.c), &value2);
					
					if(!satisfied)
						it_result_rid = result_rid.erase(it_result_rid_temp);
					else 
						it_result_rid++;
				}
			}		
		}
	}
	//get the data according to the rid
	/*for(it_result_rid = result_rid.begin(); it_result_rid != result_rid.end(); it_result_rid++){
		fhd.GetRec(rid, rec);
		char * data;
		rec.GetData(data);
		result.push_back(data);
	}*/
	cout << "MiniSQL::Select " << result_rid.size() << " row(s)." << endl; 
	for(it_attri = attributes.begin(); it_attri != attributes.end(); it_attri++){
		string temp;
		temp = it_attri->name;
		if(temp.size() < 20){
			temp.append(20 - temp.size(), ' ');
		}
		cout << temp << "|";
	}
	cout << endl;
	for(it_result_rid = result_rid.begin(); it_result_rid != result_rid.end(); ++it_result_rid){
		string temp;
		int pivot = 0;
		for (it_attri = attributes.begin(); it_attri != attributes.end(); it_attri++){
			fhd.GetRec(* it_result_rid, rec);
			char * data;
			rec.GetData(data);
			if (it_attri->type == Package::INT_){
				int i;
				memcpy((int *)&i, (data) + pivot, it_attri->size);		
				pivot += it_attri->size;
				stringstream stream;
				stream << i;
				stream >> temp;
			}
			else if(it_attri->type == Package::FLOAT_){
				float f;
				memcpy((float *)&f, data + pivot, it_attri->size);
				pivot += it_attri->size;
				stringstream stream;
				stream << f;
				stream >> temp;
			}
			else{
				char * temp_c;
				temp_c = (char *)malloc(sizeof(char) * it_attri->size);
				memcpy(temp_c, data + pivot, it_attri->size);
				pivot += it_attri->size;
				temp = temp_c;
			}
			if(temp.size() < 20)
				temp.append(20 - temp.size(), ' ');
			cout << temp << "|";
		}
		cout << endl;
	}
	record_m->CloseFile(fhd);
}
