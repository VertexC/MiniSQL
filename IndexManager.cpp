#include "IndexManager.h"

//------------------------function of Index Manager-------------------------//
//
//to convert the string type key into actual type key according to the type
//f=
void Index_Manager::set_key(int type, string key){
	stringstream stream;
	stream << key;
	if(type == - 1)//int-type
		stream >> int_key;
	else if(type == 0)//float-type
		stream >> float_key;
	else if(type > 0)//string-type
		stream >> string_key;
	else{
		cout << "Error: Index_Manager::set_key(int type, string key). Invalid type of key." << endl;
	}
}

//
//get the degree according to the block size
//bug:the port of block size is not varified
int Index_Manager::get_degree(int type){
	return (BF_BLOCK_SIZE - 50)/((get_key_size(type) + sizeof(offset)));
}

//
//to get the keysize
//for string, the value of type is the sizeof string 
int Index_Manager::get_key_size(int type){
	if(type == -1)//int-type
		return sizeof(int);
	else if(type == 0)
		return sizeof(float);
	else if(type > 0)//type is the size of string
		return type + 1;//string.size + '\0'
	else{
		cout << "Error:Index_Manager::get_key_size(int type). Invalid type." << endl;
	}
}


//
//create index
//bug:not tested
void Index_Manager::create_Index(const char * file_ptr, int type){
	int keysize = get_key_size(type);
	int m_degree = get_degree(type);
	if(type == -1){//int
		//blockf_manager.createFile(file_ptr);
		//blockf_manager.openFile(file_ptr);
		Bplus_Tree<int> *Tree = new Bplus_Tree<int>(file_ptr, keysize, type, m_degree);
		delete Tree;
	}
	else if(type == 0){//float
		//blockf_manager.createFile(file_ptr);
		//blockf_manager.openFile(file_ptr);
		Bplus_Tree<float> * Tree = new Bplus_Tree<float>(file_ptr, keysize, type, m_degree);
		delete Tree;
	}
	else if(type > 0){//string
		//blockf_manager.createFile(file_ptr);
		Bplus_Tree<string> * Tree = new Bplus_Tree<string>(file_ptr, keysize, type, m_degree);
		delete Tree;
	}
	//blockf_manager.closeFile(file_ptr);
}

//
//drop index
//bug:not tested
void Index_Manager::drop_Index(const char * file_ptr){
	/*if (type == 0)//int type
	{
		blockf_manager.removeFile(file_ptr);
	}
	else if (type == 1)//float type
	{
		blockf_manager.removeFile(file_ptr);
	}
	else if (type > 0){
		blockf_manager.removeFile(file_ptr);
	}
	else{
		cout << "Index_Manager::drop_Index(string file_ptr, int type). Invalid type." << endl;
	}*/
	blockf_manager.removeFile(file_ptr);
}


//
//search index
//bug:not tested
offset Index_Manager::search_Index(const char * file_ptr, string key, int type){
	set_key(type, key);
	offset result;
	if (type == - 1)//int type
	{
		Bplus_Tree<int>* Tree = new Bplus_Tree<int>(file_ptr);
		Tree->search_key(int_key, result);
		delete Tree;
		return result;
	}
	else if (type == 0)//float type
	{
		//blockf_manager.openFile(file_ptr);
		Bplus_Tree<float>* Tree = new Bplus_Tree<float>(file_ptr);
		Tree->search_key(float_key, result);
		delete Tree;
		return result;
	}
	else if (type > 0)//string type
	{
		//blockf_manager.openFile(file_ptr);
		Bplus_Tree<string>* Tree = new Bplus_Tree<string>(file_ptr);
		Tree->search_key(string_key, result);
		delete Tree;
		return result;
	}
	else{
		cout << "Error:Index_Manager::search_Index(string file_ptr, string key, int type). Invalid type." << endl;
	}
	//blockf_manager.closeFile(file_ptr);
}

//
//delete a index
//bug:not tested
void Index_Manager::delete_Index(const char * file_ptr, string key, int type){
	set_key(type, key);
	if (type == -1)//int type
	{
		Bplus_Tree<int>* Tree = new Bplus_Tree<int>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		Tree->delete_key(int_key);
		delete Tree;
	}
	else if (type == 0)//float type
	{
		Bplus_Tree<float>* Tree = new Bplus_Tree<float>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		Tree->delete_key(float_key);
		delete Tree;
	}
	else if (type > 0){//string type
		Bplus_Tree<string>* Tree = new Bplus_Tree<string>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		Tree->delete_key(string_key);
		delete Tree;
	}
	else{
		cout << "Error:Index_Manager::delete_Index(string file_ptr, string key, int type). Invalid type." << endl;
	}
	//blockf_manager.closeFile(file_ptr);
}

void Index_Manager::delete_Index(const char * file_ptr, offset address, int type){
	//set_key(type, key);
	if (type == -1)//int type
	{
		Bplus_Tree<int>* Tree = new Bplus_Tree<int>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		Tree->delete_key(address);
		delete Tree;
	}
	else if (type == 0)//float type
	{
		Bplus_Tree<float>* Tree = new Bplus_Tree<float>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		Tree->delete_key(address);
		delete Tree;
	}
	else if (type > 0){//string type
		Bplus_Tree<string>* Tree = new Bplus_Tree<string>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		Tree->delete_key(address);
		delete Tree;
	}
	else{
		cout << "Error:Index_Manager::delete_Index(string file_ptr, string key, int type). Invalid type." << endl;
	}
}

//
//insert a index
//bug:not tested m                                               
bool Index_Manager::insert_Index(const char * file_ptr, string key, offset address, int type){
	set_key(type, key);
	bool success = false;
	if (type == -1)//int type
	{
		Bplus_Tree<int>* Tree = new Bplus_Tree<int>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		success = Tree->insert_key(int_key, address);
		delete Tree;
	}
	else if(type == 0){//float type
		Bplus_Tree<float>* Tree = new Bplus_Tree<float>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		success = Tree->insert_key(float_key, address);
		delete Tree;
	}
	else if(type > 0){//float type
		Bplus_Tree<string>* Tree = new Bplus_Tree<string>(file_ptr);
		//blockf_manager.openFile(file_ptr);
		success = Tree->insert_key(string_key, address);
		delete Tree;
	}
	else{
		cout << "Error:insert_Index(string filename, string key, offset address).Invalid type." << endl;
	}
	return success;
	//blockf_manager.closeFile(file_ptr);
}


//
//search index
//bug:not tested
void Index_Manager::Index_search(const char * file_ptr, void * compare_value, AttrType attrType, CompOp compOp, vector<RID> & result_rid){
	
	if(attrType == INT_){
		int_key = (*(int *)compare_value);
		Bplus_Tree<int>* Tree = new Bplus_Tree<int>(file_ptr);
		if(compOp == EQ){//equal
			RID result;
			if(Tree->search_key(int_key, result)){
				result_rid.push_back(result);
			}
		}
		else if(compOp == LT || compOp == GT || compOp == LE || compOp == GE){//less than
			Tree->index_search(int_key, compOp, result_rid);
		}
		delete Tree;	
	}
	else if(attrType == FLOAT_){
		float_key = (*(float *)compare_value);
		Bplus_Tree<float>* Tree = new Bplus_Tree<float>(file_ptr);
		if(compOp == EQ){//equal
			RID result;
			if(Tree->search_key(float_key, result)){
				result_rid.push_back(result);
			}
		}
		else if(compOp == LT || compOp == GT || compOp == LE || compOp == GE){//less than
			Tree->index_search(float_key, compOp, result_rid);
		}
		delete Tree;
	}
	else{
		string_key = ((char *)compare_value);
		Bplus_Tree<string>* Tree = new Bplus_Tree<string>(file_ptr);
		if(compOp == EQ){//equal
			RID result;
			if(Tree->search_key(string_key, result)){
				result_rid.push_back(result);
			}
		}
		else if(compOp == LT || compOp == GT || compOp == LE || compOp == GE){//less than
			Tree->index_search(string_key, compOp, result_rid);
		}
		delete Tree;
	} 

}
