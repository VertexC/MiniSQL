//work for all insertion, but cannot write nodes into file
//not tested with deletion



// Minisql
// B+Tree.h
// The implementation of B+ tree
//
// Created by Chen Biwen on 2017/05/31
//
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "RM_RID.h"
#include "rm.h" 
#include "BFManager.h"
#define M_DEGREE 3//the degree of node
using namespace std;
//--------TreeNode-------------------//
//keytype: int, float, string

typedef RID offset;//offset of the address of the whole tuple
typedef int blocknum;
//every tree node is read from block
//if new a node, allocate form buffermanager
//else get the block from buffermanager
//the structure of data is
//m_degree(4) node_number(4) is_leaf(1) type(4) key_size(4) key_cout(4) [keys](key_size)  [children](4)/[address_of_tuple](4)
static BFManager blockf_manager;
template <typename keytype>
class TreeNode{
private:
	BF_FileHandle * blockf_file_handler_ptr;
	BF_BlockHandle blockf_block_handler;
public:
	blocknum node_number;
	bool is_leaf;
	int key_count;//the total number of key
	blocknum parent;
	vector<blocknum> children;
	vector<keytype> keys;//the value of key
	vector<offset> address_of_tuple;
	blocknum next_leaf_node;
private:
	int m_degree;
	int key_size;
	int type;
    const char * file_name = NULL;
	char * block_data_ptr = NULL;
public:
	TreeNode(bool is_leaf, int m_degree , int key_size, int type, const char * file_name, BF_FileHandle * blockf_file_handler_ptr);
	TreeNode(const char * file, blocknum number, BF_FileHandle * blockf_file_handler_ptr);
	TreeNode(const TreeNode<keytype> & copy);
	TreeNode(){}
	~TreeNode();
public:
	void write_back_to_block();
	bool is_root();
	bool find_in_node(keytype key, size_t & index);//find a key in node, return the position as & index, and whter find the key
	int add_key(keytype key, int buffer_in);//insert a (key,children) 
	int add_key(keytype key, offset buffer_in);//insert key,rid
	void remove_key(keytype key, bool before);//remove a key in the treenode
	void drop_node(blocknum number);
	void replace(const char *file, blocknum number, BF_FileHandle * blockf_file_handler_ptr);
	void mark_dirty();
	void unpin();
	void Print();
};

template <class keytype>
TreeNode<keytype>::TreeNode
(bool is_leaf, int m_degree , int key_size, int type, const char * file_name, BF_FileHandle * blockf_file_handler_ptr)
:is_leaf(is_leaf), m_degree(m_degree), key_size(key_size), type(type), file_name(file_name), blockf_file_handler_ptr(blockf_file_handler_ptr)
{
	for (size_t i = 0; i < m_degree - 1; ++i){
		children.push_back(-1);
		keys.push_back(keytype());
		address_of_tuple.push_back(offset());
    }
    children.push_back(blocknum());
	if(blockf_file_handler_ptr->AllocateBlock(blockf_block_handler) >= 0)
		//cout << "Allocate a new Block:";
	blockf_block_handler.getBlockNum(node_number);
	blockf_block_handler.getData(block_data_ptr);
	//cout << node_number << endl;
	key_count = 0;
	parent = -1;
	next_leaf_node = -1;
}



template <class keytype>
TreeNode<keytype>::TreeNode(const char * file, blocknum number, BF_FileHandle * blockf_file_handler_in){
	file_name = file;
	node_number = number;
	blockf_file_handler_ptr = blockf_file_handler_in;
	blockf_file_handler_ptr->GetThisBlock(node_number, blockf_block_handler);
		//cout << "Get a old Block:" << number << endl;
	/*else {
		////cout << "faile" ; 
		//cout << blockf_file_handler_ptr->GetThisBlock(node_number, blockf_block_handler);
	}*/
	blockf_block_handler.getData(block_data_ptr);
	char * data = block_data_ptr;
	//m_degree() node_number(4) is_leaf(1) next_leaf_node(4) parent(4) type(4) key_size(4) key_cout(4) [keys](key_size) [address of tuple](4) / [children](4)
	m_degree = *(int *)(data);
	data += sizeof(int); 
	////cout << "read in m_degree" << m_degree << endl;
	for (size_t i = 0; i < m_degree - 1; ++i){
			children.push_back(-1);
			keys.push_back(keytype());
			address_of_tuple.push_back(offset());
    }
    children.push_back(blocknum());
    node_number = *(int *)(data);
    data += sizeof(int);
	////cout << "read in node number" << node_number << endl;
    is_leaf = *(bool *)(data);
    data += sizeof(bool);
	////cout << "read in is_leaf" << is_leaf << endl;
    next_leaf_node = *(blocknum *)(data);
    data += sizeof(blocknum);
	////cout << "next_leaf_node" << next_leaf_node << endl;
    parent = *(blocknum *)(data);
    data += sizeof(blocknum);

    type = *(int *)(data);
    data += sizeof(int);
    //cout << "read_in_type" << type << endl;
    
    key_size = *(int *)(data);
    data += sizeof(int);

    key_count = *(int *)(data);
    data += sizeof(int);
	////cout << "read_in_key_count" << key_count << endl;
    if (type <= 0)//int float
    {
     	for (int i = 0; i < key_count; ++i)
    	{
    		keys[i] = *(keytype *)(data);
    		data += key_size;
    	}   	
    }
    else{//string
    	for(int i = 0; i < key_count; ++i)
    	{
    		char * c;
    		c = (char *)malloc(sizeof(char) * key_size);
    		char * temp;
    		////cout << key_size << endl;
    		memcpy(c, data, key_size);
    		////cout << "read in char " << c;//
    		//keys[i].erase(keys[i].begin(), keys[i].end());
    		stringstream stream;
			string key_temp = c;
			if(key_temp.length() > key_size){
				key_temp.erase(key_temp.end() - (key_temp.length() - key_size), key_temp.end());
			}
			stream << key_temp;	
			stream >> keys[i];
    		////cout << "read in keys " << keys[i] << endl; 
    		data += key_size;
    	}
    }

    if (is_leaf)
    {
    	for (int i = 0; i < key_count; ++i)
    		{
    			address_of_tuple[i] = *(offset *)(data);
    			data += sizeof(offset);
    		}	
    }
    else{
    	for (int i = 0; i <= key_count; ++i)
    	{
    		children[i] = *(blocknum *)(data);
    		data += sizeof(blocknum);
    	}
    }
}

template <class keytype>
TreeNode<keytype>::TreeNode(const TreeNode<keytype> & copy){
	node_number = copy.node_number;
	key_count = copy.key_count;//the total number of key
	is_leaf = copy.leaf;
	parent = copy.parent;//pointer to parent
	next_leaf_node = copy.next_leaf_node;
	children.assign(copy.children.begin(), copy.children.end());
	keys.assign(copy.keys.begin(), copy.keys.end());//the value of key
	address_of_tuple.assign(copy.address_of_tuple.begin(), copy.address_of_tuple.end());
	m_degree = copy.m_degree;
	key_size = copy.key_size;
	type = copy.type;
	file_name = copy.file_name;
	blockf_file_handler_ptr = copy.blockf_file_handler_ptr;
	blockf_block_handler = copy.blockf_block_handler;
	block_data_ptr = copy.block_data_ptr;
}

template <class keytype>
TreeNode<keytype>::~TreeNode(){
	//write_back_to_block();
}


//bugs: does not work for string type
//deal with the string case specially
template <class keytype>
void TreeNode<keytype>::write_back_to_block(){
	//write the head information back_to_block
	////cout << "Node" << node_number << "write back begin" << endl;
	int pivot = 0;
	//m_degree() node_number(4) is_leaf(1) next_leaf_node(4) parent(4) type(4) key_size(4) key_cout(4) [keys](key_size) [address of tuple](4) / [children](4)
	memcpy(block_data_ptr + pivot, (int *)&m_degree, sizeof(int));
	pivot += sizeof(int);
	memcpy(block_data_ptr + pivot, (int *)&node_number, sizeof(int));
	pivot += sizeof(int);
	memcpy(block_data_ptr + pivot, (bool *)&is_leaf, sizeof(bool));
	pivot += sizeof(bool);
	memcpy(block_data_ptr + pivot, (int *)&next_leaf_node, sizeof(int));
	pivot += sizeof(int);
	memcpy(block_data_ptr + pivot, (int *)&parent, sizeof(int));
	pivot += sizeof(int);
	memcpy(block_data_ptr + pivot, (int *)&type, sizeof(int));
	pivot += sizeof(int);
	memcpy(block_data_ptr + pivot, (int *)&key_size, sizeof(int));
	pivot += sizeof(int);
	memcpy(block_data_ptr + pivot, (int *)&key_count, sizeof(int));
	pivot += sizeof(int);
	if (type <= 0)//int key // float key
	{
		for (int i = 0; i < key_count; ++i)
		{
			memcpy(block_data_ptr + pivot, (keytype *)&keys[i], key_size);
			pivot += key_size;
		}
		
	}
	else{//string type
		for (int i = 0; i < key_count; ++i){
			stringstream stream;
			string key_temp;
			stream << keys[i];
			stream >> key_temp;
			//append string to keysize
			size_t key_length = key_temp.length();
			////cout << "key_length" << key_length << endl;
			////cout << "key_size" << key_size << endl;
			if(key_size - key_length > 0)
				key_temp.append(key_size - key_length, ' ');
			else{
				key_temp.erase(key_temp.end() - (key_size - key_length), key_temp.end());
			}
			char * c;
			c = (char *)malloc(key_size);
			char * temp;
			temp = (char *)malloc(key_size);
			//not add '/0' into memory
			key_temp.copy(temp, key_size);
			for (int j = 0; j < key_size; ++j)
			{
				c[j] = temp[j];
			}
			////cout << "C char to be wirriten back" << c << "end" << endl;
			memcpy(block_data_ptr + pivot, c, key_size); 
			pivot = pivot + key_size;
		}
	}
	if(is_leaf){
		for (int i = 0; i < key_count; ++i)
		{	
			memcpy(block_data_ptr + pivot, (offset *)&address_of_tuple[i], sizeof(offset));
			pivot += sizeof(offset);
		}
	}
	else{
		for (int i = 0; i <= key_count; ++i)
		{
			memcpy(block_data_ptr + pivot, (blocknum *)&children[i], sizeof(blocknum));
			pivot += sizeof(blocknum);
		}
	}
}

template <class keytype>
bool TreeNode<keytype>::is_root(){
	if(parent == -1) return true;
	else return false;
}
template <class keytype>
bool TreeNode<keytype>::find_in_node(keytype key, size_t & index){
	if(key_count == 0){//no keys in the node
		index = 0;
		return false;
	}
	else{
		//use binary search  to find the key
		size_t left = 0;
		size_t right = key_count - 1;
		size_t pivot = 0;
		while(right > left + 1){
			pivot = (right + left) / 2;
			if(keys[pivot] == key){
				//find
				index = pivot;
				return true;
			}
			else if(keys[pivot] < key){
				left = pivot;
				continue;
			}
			else if(keys[pivot] > key){
				right = pivot;
				continue;
			}
		}
		//right == left + 1
		if (keys[left] >= key)
		{
			index = left;
			return (keys[left] == key);
		}
		else if (keys[right] >= key)
		{
			index = right;
			return (keys[right] == key);
		}
		else{
			index = right + 1;
			return false;
		}
	}
}
//insert the (key,address) into a none-leaf Node
//input:key, ptr_to_child
//return:the index of pisition inserted
//bugs:
//
template <class keytype>
int TreeNode<keytype>::add_key(keytype key, int buffer_in){
	if(!is_leaf){
		blocknum child = (blocknum)buffer_in;
		if(key_count == 0){//no key
			keys[0] = key;
			children[1] = child;
			key_count ++;
			return 0;
		}
		else{
			size_t index = 0;
			bool exist;
			exist = find_in_node(key, index);
			if(exist){
				//cout << "Error: TreeNode<keytype>::add_key(keytype & key). The value inserted into the node is alread existed." << endl;
				//exit();
			}
			else{
				//shift the value after index(index included) right 1 position
				for (int i = key_count; i > index; --i)
				{
					keys[i] = keys[i - 1];
					children[i + 1] = children[i];
				}
				keys[index] = key;
				children[index + 1] = child;
				key_count ++;
				return index;
			}
		}
	}

}

template<class keytype>
int TreeNode<keytype>::add_key(keytype key, offset buffer_in){
	//for non leaf node
	if(is_leaf){
		//cout << "add_key_in_leaf" << endl;
		offset address = (offset)buffer_in;
		if(key_count == 0){
			keys[0] = key;
			address_of_tuple[0] = address;
			key_count ++; 
			return 0;
		}
		else{
			size_t index = 0;
			bool exist;
			exist = find_in_node(key, index);
			if(exist){
				//cout << "Error: TreeNode<keytype>::add_key(keytype & key, offset address). The value inserted into the node is alread existed." << endl;
			}
			else{
				for (int i = key_count; i > index; --i)
				{
					keys[i] = keys[i - 1];
					address_of_tuple[i] = address_of_tuple[i - 1];
				}

				keys[index] = key;
				address_of_tuple[index] = address;
				key_count ++;
				return index;
			}
		}
	}
}

//remove the key in the tree node
//input:key, address
//return:whether deleted successfully
//bugs:not-completed
//
template <class keytype>
void TreeNode<keytype>::remove_key(keytype key, bool before){
	size_t index = 0;
	int find_flag = 0;
	find_flag = find_in_node(key, index);
	if(find_flag == 0){
		//cout << "Error: TreeNode<keytype>remove_key(keytype key). The key to be deleted is not in the node." << endl;
	}
	if(is_leaf){
		for (int i = index; i < key_count - 1; ++i)
		{
			keys[i] = keys[i + 1];
			address_of_tuple[i] = address_of_tuple[i + 1];
		}
		keys[key_count - 1] = keytype();
		address_of_tuple[key_count - 1] = offset();
		key_count --;
	}
	else{//none-leaf node
        if(before){
            //cout << "before !!" << endl;
            for (int i = index; i < key_count - 1; ++i)
            {
                keys[i] = keys[i + 1];
            }
            for (int i = index; i < key_count; ++i){
                children[i] = children[i + 1];
            }
            keys[key_count - 1] = keytype();
            children[key_count] = -1;
            key_count --;
        }
        else{
            for (int i = index; i < key_count - 1; ++i)
            {
                keys[i] = keys[i + 1];
                children[i + 1] = children[i + 2];
            }
            keys[key_count - 1] = keytype();
            children[key_count] = -1;
            key_count --;
        }
	}
}
//
//replace the Node by another block
//input: file_ptr, block number
//
template <class keytype>
void TreeNode<keytype>::replace(const char * filename, blocknum number, BF_FileHandle * blockf_file_handler_in){
	if(blockf_file_handler_ptr != NULL)
		unpin();//unpin the former block
	blockf_file_handler_ptr = blockf_file_handler_in;
	file_name = filename;
	//cout << "replace by block:";
	if(blockf_file_handler_ptr->GetThisBlock(number, blockf_block_handler) >= 0)
		//cout << number << endl;
	blockf_block_handler.getData(block_data_ptr);
	char * data = block_data_ptr;
	
    //m_degree() node_number(4) is_leaf(1) next_leaf_node(4) parent(4) type(4) key_size(4) key_cout(4) [keys](key_size) [address of tuple](4) / [children](4)
	m_degree = *(int*)(data);
    data += sizeof(int);
	for (size_t i = 0; i < m_degree - 1; ++i){
		children.push_back(-1);
		keys.push_back(keytype());
		address_of_tuple.push_back(offset());
    }
    children.push_back(-1);
    
	node_number = *(int *)(data);
    data += sizeof(int);

    is_leaf = *(bool *)(data);
    data += sizeof(bool);

    next_leaf_node = *(blocknum *)(data);
    data += sizeof(blocknum);

    parent = *(blocknum *)(data);
    data += sizeof(blocknum);

    type = *(int *)(data);
    data += sizeof(int);
    
    key_size = *(int *)(data);
    data += sizeof(int);

    key_count = *(int *)(data);
    data += sizeof(int);

    if (type <= 0)//int float
    {
     	for (int i = 0; i < key_count; ++i)
    	{
    		keys[i] = *(keytype *)(data);
    		data += key_size;
    	}   	
    }
    else{//string
    	for (int i = 0; i < key_count; ++i)
    	{
    		char * c;
    		c = (char *)malloc(sizeof(char) * key_size);
    		char * temp;
    		////cout << key_size << endl;
    		memcpy(c, data, key_size);
    		
			stringstream stream;
			string key_temp;
			key_temp = c;
			if(key_size - key_temp.length() > 0)
				key_temp.append(key_size - key_temp.length(), ' ');
			stream << key_temp;
			stream >> keys[i]; 

		
			////cout << "read in keys[i] " << keys[i] << endl;
    		////cout << "May be wrong " << keys[i].length() << endl; 
    		data += key_size;
    	}
    }

    if (is_leaf)
    {
    	for (int i = 0; i < key_count; ++i)
    		{
    			address_of_tuple[i] = *(offset *)(data);
    			data += sizeof(offset);
    		}	
    }
    else{
    	for (int i = 0; i <= key_count; ++i)
    	{
    		children[i] = *(blocknum *)(data);
    		data += sizeof(blocknum);
    	}
    }
    //cout << "end replace" << endl;
}

template <class keytype>
void TreeNode<keytype>::mark_dirty(){
	////cout << "MarkDirty " << node_number;
    blockf_file_handler_ptr->MarkDirty(node_number);
}

template <class keytype>
void TreeNode<keytype>::unpin(){
	mark_dirty();
	write_back_to_block();
	blockf_file_handler_ptr->UnpinBlock(node_number);
}

//Print the Node, for test
//input:void
//bugs:none
//
template <class keytype>
void TreeNode<keytype>::Print(){
	int i;
    for( i = 0; i < key_count; i++){
        //cout << " key " << keys[i];
        if (is_leaf)
        {
        	////cout << " offset " << address_of_tuple[i];
        	//cout << " rid ";
        }
        else{
        	//cout << " child" << children[i];
        }
    }
    if (!is_leaf)
    {
    	//cout << "child" << children[i];
    }
    //cout << endl;
};

//---------BplusTree------------------------//
//keytype: int, float, char
template <typename keytype>
class Bplus_Tree{

friend class BFManager;
friend class BF_FileHandle;
friend class BF_BlockHnadle;
private:
    BF_FileHandle  blockf_file_handler;
	BF_FileHandle * blockf_file_handler_ptr; 
	BF_BlockHandle blockf_block_handler;
private:
	typedef TreeNode<keytype> * Node;
	//for find the key
	struct search_Tree_result{
		Node pNode;//pointer the the finded node
		size_t index;//the position of the key in the index
		bool is_found;//the flag of searching
	};
private:
	const char * file_name;
	//blocknum Tree_block_head;//the block head of the tree
	char * Tree_block_data;
	int leaf_key_count;//total key of the tree
	int node_count; //the number of nodes
	int type;//keytype
	int key_size;
	int key_count;//key count of tree
	int m_degree;
public:
    //Node root;
    int root_number;//block number of root
	//Node leaf_head;//head of leaf
	int leaf_head_number;//block number of first leaf head
	Bplus_Tree();
	Bplus_Tree(const char * file_name, int key_size, int type, int m_degree);
	Bplus_Tree(const char * file_name);
	Bplus_Tree(const Bplus_Tree<keytype> & copy);
	~Bplus_Tree();
public:
	void search_key(keytype key, search_Tree_result & snp);//search the key, return index and Node
	bool search_key(keytype key, offset & address);
	bool insert_key(keytype key, offset address);
	void insert_in_parent(Node N, keytype key, Node N_temp);
    void index_search(keytype key, CompOp compOp, vector<offset> & result_offset);
	void delete_key(keytype key);
    void delete_key(offset address);
	void delete_entry(keytype key, Node N, bool before);
	void drop_Tree(Node node);//delete all the tree nodes, used in ~
	void write_back_to_block();
	void read_from_disk();
	void Print_Tree();//for debug;
	void Print_Node(blocknum number);//for debug
	bool is_empty();
};


template <class keytype>
void Bplus_Tree<keytype>::index_search(keytype key, CompOp compOp, vector<offset> & result_offset){
	if(is_empty())
		return;

	search_Tree_result snp;
	search_key(key, snp);
	snp.pNode->unpin();
	int modify;
	if(compOp == LT){
		//less than
		int number = 0;
		number = leaf_head_number;
		Node T = new TreeNode<keytype>(1, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		while(1){
			T->replace(file_name, number, blockf_file_handler_ptr);
			if(T->node_number == snp.pNode->node_number){

				if(snp.is_found)modify = -1;
				else modify = -1;
				if(snp.index + modify < 0) break;
				for(int i = 0; i <= snp.index + modify; i++){
					result_offset.push_back(T->address_of_tuple[i]);
				}
				break;
			}
			else{
				for(int i = 0; i < T->key_count; i++){
					result_offset.push_back(T->address_of_tuple[i]);
				}
			}
			number = T->next_leaf_node;
		}
		T->unpin();
	}
	else if(compOp == LE){
		//less equal than
		int number = 0;
		number = leaf_head_number;
		Node T = new TreeNode<keytype>(1, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		while(1){
			T->replace(file_name, number, blockf_file_handler_ptr);
			if(T->node_number == snp.pNode->node_number){

				if(snp.is_found)modify = 0;
				else modify = -1;
				if(snp.index + modify < 0) break;
				for(int i = 0; i <= snp.index + modify; i++){
					result_offset.push_back(T->address_of_tuple[i]);
				}
				break;
			}
			else{
				for(int i = 0; i < T->key_count; i++){
					result_offset.push_back(T->address_of_tuple[i]);
				}
			}
			number = T->next_leaf_node;
		}
		T->unpin();
	}
	else if(compOp == GT){
		//greater than
		int number = 0;
		number = leaf_head_number;
		Node T = new TreeNode<keytype>(1, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		number = snp.pNode->node_number;
		while(1){
			T->replace(file_name, number, blockf_file_handler_ptr);
			if(T->node_number == snp.pNode->node_number){
				if(snp.is_found)modify = 1;
				else modify = 0;
				if(snp.index + modify > T->key_count - 1) break;
				for(int i = snp.index + modify; i < T->key_count; i++){
					result_offset.push_back(T->address_of_tuple[i]);
				}
			}
			else{
				for(int i = 0; i < T->key_count; i++){
					result_offset.push_back(T->address_of_tuple[i]);
				}
			}
			number = T->next_leaf_node;
			if(number == -1)
			break;
		}
		T->unpin();
	}
	else if(compOp == GE){
		//greater than
		int number = 0;
		number = leaf_head_number;
		Node T = new TreeNode<keytype>(1, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		number = snp.pNode->node_number;
		while(1){
			T->replace(file_name, number, blockf_file_handler_ptr);
			if(T->node_number == snp.pNode->node_number){
				if(snp.is_found)modify = 0;
				else modify = 0;
				if(snp.index + modify > T->key_count - 1) break;
				for(int i = snp.index + modify; i < T->key_count; i++){
					result_offset.push_back(T->address_of_tuple[i]);
				}
			}
			else{
				for(int i = 0; i < T->key_count; i++){
					result_offset.push_back(T->address_of_tuple[i]);
				}
			}
			number = T->next_leaf_node;
			if(number == -1)
			break;
		}
		T->unpin();
	}
}

//Bplus_Tree default construction 
//
//
//
template <class keytype>
Bplus_Tree<keytype>::Bplus_Tree(){

}

//Bplus_Tree construction for read Node from block
//
//
template <class keytype>
Bplus_Tree<keytype>::Bplus_Tree(const char * filename){
	file_name = filename;
	//cout << "Bplus_Tree(const char * file_name)" << endl;
	//blockf_manager.createFile(file_name);
	//initialize the file handler
	//cout << "filename" << file_name << endl;
	//cout << "Blockf_file_handler" 
	blockf_manager.openFile(file_name, blockf_file_handler);
	//block handler for Tree head(0 as Tree_head_number)
	blockf_file_handler_ptr = (BF_FileHandle *) & blockf_file_handler;
	//cout << "Get_this_block" << 
	blockf_file_handler_ptr->GetThisBlock(0, blockf_block_handler);
	blockf_block_handler.getData(Tree_block_data);
	//node_count(4) leaf_key_count(4) root_number(4) leaf_head_number(4) key_size(4) type(4) m_degree(4)
	////cout << Tree_block_data << endl;
	char * data = Tree_block_data;
	node_count = *(int *)data;
	//cout << "node_cout" << node_count << endl;
	data += sizeof(int);
	leaf_key_count = *(int *)data;
	//cout << "leaf_key_count" << leaf_key_count << endl;
	data += sizeof(int);
	root_number = *(blocknum *)data;
	//cout << "root_number" << root_number << endl;
	/*if(root_number != -1){
		root = new TreeNode<keytype>(file_name, root_number, blockf_file_handler->ptr);
	}*/
	data += sizeof(blocknum);
	leaf_head_number = *(blocknum *)data;
	//cout << "leaf_head_number" << leaf_head_number << endl;
	/*if(leaf_head_number != -1){
		leaf_head = new TreeNode<keytype>(file_name, leaf_head_number, blockf_file_handler->ptr);
	}*/
	data += sizeof(blocknum);
	key_size = *(int *)data;
	//cout << "key_size" << key_size << endl;
	data += sizeof(int);
	type = *(int *)data;
	//cout << "type" << type << endl;
	data += sizeof(int);
	m_degree = *(int *)data;
	//cout << "m_degree" << m_degree << endl;
	data += sizeof(int);
}

//Bplus_Tree construction for new a Node
//input:file_ptr, key_size type, m_degree
//bugs:not-completed
//
template <class keytype>
Bplus_Tree<keytype>::Bplus_Tree(const char * filename, int key_size, int type, int m_degree)
:key_size(key_size), type(type), m_degree(m_degree)
{
	//cout << "Bplus_Tree(const char * filename, int key_size, int type, int m_degree)" << endl;
	file_name = filename; 
	node_count = 0;
	//root = NULL;
	root_number = -1;
	//leaf_head = NULL;
	leaf_head_number = -1;
	Tree_block_data = NULL;
	leaf_key_count = 0;
	blockf_manager.createFile(file_name);
	blockf_manager.openFile(file_name, blockf_file_handler);
	blockf_file_handler_ptr = (BF_FileHandle *)&blockf_file_handler;
	//Alocate the Tree_head;
	//0 is the block number of head
	blockf_file_handler_ptr->AllocateBlock(blockf_block_handler);
	blockf_block_handler.getData(Tree_block_data);
	//node_count(4) leaf_key_count(4) root_number(4) leaf_head(4) key_size(4) type(4) m_degree(4)
}
//Bplus_Tree deconstruction
//input:default
//bugs:not-completed
//
template <class keytype>
Bplus_Tree<keytype>::~Bplus_Tree(){
	//cout << "~Bplus_Tree()" << endl;
	write_back_to_block();
	blockf_manager.closeFile(blockf_file_handler);
	//cout << "~end Bplus_Tree()" << endl;
}
//write back to block
//input:void
//bugs:not-completed
//
template <class keytype>
void Bplus_Tree<keytype>::write_back_to_block(){
	//node_count(4) leaf_key_count(4) root_number(4) leaf_head(4) key_size(4) type(4) m_degree(4)
	//cout << "wire_back_to_block()" << endl;
	int pivot = 0;
	//cout << node_count << endl;
	memcpy(Tree_block_data + pivot, (int *)&node_count, sizeof(int));
	pivot += sizeof(int);
	//cout << leaf_key_count << endl;
	memcpy(Tree_block_data + pivot, (int *)&leaf_key_count, sizeof(int));
	pivot += sizeof(int);
    //cout << root_number << endl;
	memcpy(Tree_block_data + pivot, (blocknum *)&root_number, sizeof(blocknum));
	pivot += sizeof(blocknum);
	//cout << leaf_head_number << endl;
	memcpy(Tree_block_data + pivot, (blocknum *)&leaf_head_number, sizeof(blocknum));
	pivot += sizeof(blocknum);
	//cout << key_size << endl;
	memcpy(Tree_block_data + pivot, (int *)&key_size, sizeof(int));
	pivot += sizeof(int);
	//cout << "type" << type << endl;
	memcpy(Tree_block_data + pivot, (int *)&type, sizeof(int));
	pivot += sizeof(int);
	//cout << m_degree << endl;
	memcpy(Tree_block_data + pivot, (int *)&m_degree, sizeof(int));
	pivot += sizeof(int);
	//cout << "Dirty " << 
	blockf_file_handler_ptr->MarkDirty(0);
	//cout << "Upin " << 
	blockf_file_handler_ptr->UnpinBlock(0);
}

//Bplus_Tree copy - construction
//input:default
//bugs:tested
//
template <class keytype>
Bplus_Tree<keytype>::Bplus_Tree(const Bplus_Tree<keytype> & copy){

	file_name = copy.file_name;
	Tree_block_data = copy.Tree_block_data;
	node_count = copy.node_count;
	type = copy.type;
	key_size = copy.key_size;
	m_degree = copy.m_degree;
	//root = copy.root;
	root_number = copy.root_number;
	//leaf_head = copy.leaf_head;//head of leaf
	leaf_head_number = copy.leaf_head_number;

}
//input:key
//return: The node of the search key, and the index.
//if there is no such key, return the key which it should be and index
//bugs:tested
//
template <class keytype>
void Bplus_Tree<keytype>::search_key(keytype key, search_Tree_result & snp){
	Node C = new TreeNode<keytype>(file_name, root_number, blockf_file_handler_ptr);
	//while c is not leaf
	while(C->is_leaf != 1){
		int i = 0;
		//find smallest i such V <= C.Ki
		for (i = 0; i < C->key_count; ++i)
		{
			if(C->keys[i] >= key)
				break;
		}
		if(i == C->key_count){//no such i, key is bigger than all the keys
			C->replace(file_name, C->children[C->key_count], blockf_file_handler_ptr);
		}
		else if (key == C->keys[i])
		{
			C->replace(file_name, C->children[i + 1], blockf_file_handler_ptr);
		}
		else // key < C.keys[i]
			C->replace(file_name, C->children[i], blockf_file_handler_ptr);
	}
	//C is a leaf node
	snp.is_found = C->find_in_node(key, snp.index);
	C->unpin();
	snp.pNode = new TreeNode<keytype>(file_name, C->node_number, blockf_file_handler_ptr);
}

//input:key
//return: The offset of the key
//if there is no such key, return -1
//bugs:untested
//
template <class keytype>
bool Bplus_Tree<keytype>::search_key(keytype key, offset & address){
	Node C = new TreeNode<keytype>(file_name, root_number, blockf_file_handler_ptr);
	//while c is not leaf
	while(C->is_leaf != 1){
		int i = 0;
		//find smallest i such V <= C.Ki
		for (i = 0; i < C->key_count; ++i)
		{
			if(C->keys[i] >= key)
				break;
		}
		if(i == C->key_count){//no such i, key is bigger than all the keys
			C->replace(file_name, C->children[C->key_count], blockf_file_handler_ptr);
		}
		else if (key == C->keys[i])
		{
			C->replace(file_name, C->children[i + 1], blockf_file_handler_ptr);
		}
		else // key < C.keys[i]
			C->replace(file_name, C->children[i], blockf_file_handler_ptr);
	}
	//C is a leaf node
	size_t index;
	bool find_flag;
	find_flag = C->find_in_node(key, index);
	address = C->address_of_tuple[index];
	return find_flag;
}

//input:key, address
//return: void
//
//bugs:tested
//
template <class keytype>
bool Bplus_Tree<keytype>::insert_key(keytype key, offset address){
	size_t index;
	search_Tree_result snp;
	Node L;
	//cout << "Insert_key: " << key << endl;
	if(is_empty()){
		//create an empty leaf node, which is also the root
		//cout << "Empty tree!" << endl;
		//(bool is_leaf, int m_degree , int key_size, int type, const char * file_name, BF_FileHandle & blockf_file handler)
		L = new TreeNode<keytype>(1, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		root_number = L->node_number;
		leaf_head_number = L->node_number;
		node_count ++;
	}
	else{
        //cout << "Begin search key" << endl;
        search_key(key, snp);
		if(snp.is_found){
			//cout << "Error: Bplus_T.insert_key(keytype & key, size_t & index). The inserted key is already existed." << endl;
			//exit(); 
			return false;
		}
		////cout << "end search key" << endl;
		//snp.pNode->Print();
		L = snp.pNode;
		//L->Print();
	}
	if (L->key_count < m_degree - 1)
	{
		L->add_key(key, address);
		L->unpin();
		//cout << "add_key " << key << " without split" << endl;
	}
	//rewrite flag//
	else{
		//cout << "start split" << endl;
		Node L_temp = new TreeNode<keytype>(1, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		Node T = new TreeNode<keytype>(1, m_degree + 1, key_size, type, file_name, blockf_file_handler_ptr);
		for (int i = 0; i < m_degree - 1; ++i)
		{
			T->keys[i] = L->keys[i];
			T->address_of_tuple[i] = L->address_of_tuple[i];
			T->key_count++;
		}
        T->add_key(key, address);
      
		L_temp->next_leaf_node = L->next_leaf_node;
		L->next_leaf_node = L_temp->node_number;

		//clean up the L
		L->children.assign(m_degree, -1);
		//L->keys.erase(L->keys.begin(), L->keys.end());
		L->address_of_tuple.erase(L->address_of_tuple.begin(), L->address_of_tuple.end());
		L->key_count = 0;
		for (int i = 0; i < (m_degree + 1)/2; ++i)
		{
			L->keys[i] = T->keys[i];
			L->address_of_tuple[i] = T->address_of_tuple[i];
			////cout << T->address_of_tuple[i] << endl;
            L->key_count++;
		}
		int count = 0;
		//Node Tree_5 = new TreeNode<keytype>(file_name, 2, blockf_file_handler_ptr);	
		for (int i = (m_degree + 1)/2; i < m_degree; ++i)
		{
			L_temp->keys[count] = T->keys[i];
			L_temp->address_of_tuple[count]= T->address_of_tuple[i];
			L_temp->key_count++;
			count ++;
		}
		keytype K_temp;
		K_temp = L_temp->keys[0];
		//cout << "L: " << endl; L->Print();
        //cout << "L_temp: " << endl; L_temp->Print();
		//cout << "end split" << endl;
		//T->unpin();
		blockf_file_handler_ptr->UnpinBlock(T->node_number);
		//Node Tree_1 = new TreeNode<keytype>(file_name, 2, blockf_file_handler_ptr);		
		insert_in_parent(L, K_temp, L_temp);
		node_count ++;
	}
	leaf_key_count ++;
	return true;
}
//input:Node, key, address
//return: void
//help insert function to insert in parent
//bugs:tested
//
template <class keytype>
void Bplus_Tree<keytype>::insert_in_parent(Node N, keytype key, Node N_temp){
	//cout << "insert_in_parent!" << endl;
	if(N->is_root() == 1){
        //cout << "deal with the root!" << endl;
		//creat a new root;
		Node K;
		K = new TreeNode<keytype>(0, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		root_number = K->node_number;
		K->keys[0] = key;
		K->key_count ++;
		K->children[0] = N->node_number;
		K->children[1] = N_temp->node_number;
		N->parent = K->node_number;
		N_temp->parent = K->node_number;
		N_temp->unpin();
		N->unpin();
		K->unpin();
		////cout << "return" << endl;P->key_count;
		return ;
	}
	Node P = new TreeNode<keytype>(file_name, N->parent, blockf_file_handler_ptr);
	if(P->key_count < m_degree - 1){
		//cout << "no need to redistribution" << endl;
		P->add_key(key, N_temp->node_number);
		N_temp->parent = P->node_number;
		//cout << "-----------------------------------------------" << endl; 
		N->unpin();
		N_temp->unpin();
		P->unpin();
		//add parent link in function
	}
	else{
		//cout << "need to distribution" << endl;
		Node P_temp;
		P_temp = new TreeNode<keytype>(0, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		//copy P to a block of memory T that can hold P and K
		Node T;
		
		T = new TreeNode<keytype>(0, m_degree + 1, key_size, type, file_name, blockf_file_handler_ptr);
		int i;
		for (i = 0; i < m_degree - 1; ++i)
		{
			T->keys[i] = P->keys[i];
			T->children[i] = P->children[i];
			T->key_count++;
		}
		T->children[i] = P->children[i];

		T->add_key(key, N_temp->node_number);
		N_temp->parent = T->node_number;
		//add parent link in function
		P->keys.erase(P->keys.begin(), P->keys.end());
		P->children.assign(m_degree, -1);
		P->key_count = 0;
		N_temp->unpin();
		N->unpin();
		Node Child = new TreeNode<keytype>(0, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		for (int i = 0; i < (m_degree + 1)/2; ++i)
		{
			P->children[i] = T->children[i];
			Child->replace(file_name, P->children[i], blockf_file_handler_ptr);
			Child->parent = P->node_number;
			if(i == (m_degree + 1)/2 - 1)
                break;
			P->keys[i] = T->keys[i];
			P->key_count ++;
		}
		int count = 0;
		for (int i = (m_degree + 1) / 2; i < m_degree; ++i)
		{
			P_temp->keys[count] = T->keys[i];
			P_temp->children[count] = T->children[i];
			Child->replace(file_name, P_temp->children[count], blockf_file_handler_ptr);
			Child->parent = P_temp->node_number;
			P_temp->key_count ++;
			count ++;
		}
		P_temp->children[count] = T->children[m_degree];
		Child->replace(file_name, P_temp->children[count], blockf_file_handler_ptr);
		Child->parent = P_temp->node_number;
		Child->unpin();
		//(P_temp->children[count])->parent = P_temp->node_number;
		keytype K;
		K = T->keys[(m_degree + 1)/2 - 1];
		//cout << "K" << K << endl;
		//cout << "P " << endl; P->Print();
        //cout << "p_TEMP " << endl;P_temp->Print();
		//T->unpin();
		//cout << "end insert_in_parent" << endl;
		//N_temp->write_back_to_block();
		//N->write_back_to_block();
		blockf_file_handler_ptr->UnpinBlock(T->node_number);
		insert_in_parent(P, K, P_temp);
		node_count ++;
	}
}



//input:address
//return void
//use rid to get the key value, then delete by key value
template <class keytype>
void Bplus_Tree<keytype>::delete_key(offset address){
	int number = leaf_head_number;
	Node L = new TreeNode<keytype>(1, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
	while(number != -1){
		L->replace(file_name, leaf_head_number, blockf_file_handler_ptr);
		for(int i = 0; i < L->key_count; i++){
			int block1;
			int block2;
			int slot1;
			int slot2;
			L->address_of_tuple[i].GetBlockNum(block1);
			address.GetBlockNum(block2);
			L->address_of_tuple[i].GetSlotNum(slot1);
			address.GetSlotNum(slot1);
			if(block1 == block2 && slot1 == slot2){
				//find the offset
				L->unpin();
				delete_key(L->keys[i]);
				return ;
			}
		}
		number = L->next_leaf_node;
	}
	L->unpin();
}

//input:key
//return: void
//the top of delete key
//bugs:incompleted
//
template <class keytype>
void Bplus_Tree<keytype>::delete_key(keytype key){
	struct search_Tree_result snp;
	if(type > 0){
		stringstream stream;
		stream << key;
		string key_temp;
		stream >> key_temp; 
		if(key_size - key_temp.length() > 0)
			key_temp.append(key_size - key_temp.length(), ' ');
	}
	//cout << "delete key: " << key << "end" << endl;
	search_key(key, snp);
	if(snp.is_found == 0){
		//cout << "Error. Bplus_Tree<keytype>::delete_key(keytype key). The key to be deleted is not found.";
		return;
	}
	//go to the leaf node and delete the key
	delete_entry(key, snp.pNode, 0);
	leaf_key_count --;
}

//input:key
//return: void
//the top of delete key
//bugs:what is the time to change the root?
//
template <class keytype>
void Bplus_Tree<keytype>::delete_entry(keytype key, Node N, bool before){
    //cout << "remove" << key << endl;
	N->remove_key(key, before);
	//cout << "N keycout" << N->key_count << endl;
	//what is the time to change to root?
	if (N->is_root() && N->is_leaf != 1 && (N->children[1] == -1))
	{
		//there is only one left most child
		//cout << "change the root!" << endl;
		root_number = N->children[0];
		//N->children[0]->Print();
		delete N;
		node_count --;
		return ;
	}
	else if((((N->is_leaf == 1) && (N->key_count < (m_degree)/2))
			|| ((N->is_leaf == 0) && (N->key_count + 1) < (m_degree + 1)/2))
			&& !N->is_root()){
        //cout << "The node is too small!" << endl;
		int previous_flag = 0;//1 for previous child, 0 for next child
		Node N_temp = new TreeNode<keytype>(N->is_leaf, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
		keytype K_temp;
		Node Parent = new TreeNode<keytype>(file_name, N->parent, blockf_file_handler_ptr);
		//go to the parent to find the valid children
		int i = 0;
		while(Parent->children[i] != -1){
            if(Parent->children[i] == N->node_number){
                break;
            }
            else i++;
		}
		//cout << i << endl;
		if(i == Parent->key_count + 1) //cout << "Error: Not the right parent" << endl;
		if(Parent->children[i + 1] != -1 && i + 1 <= Parent->key_count){
            N_temp->replace(file_name, Parent->children[i + 1], blockf_file_handler_ptr);
            K_temp = Parent->keys[i];
            previous_flag = 0;
		}
		else{
            N_temp->replace(file_name, Parent->children[i - 1], blockf_file_handler_ptr);
            K_temp = Parent->keys[i - 1];
            previous_flag = 1;
		}
		/*invalid find the sibling code, bugs when N has no key value but one pointer to children
		size_t index = 0;
		int find_flag = 0;
		find_flag = N->parent->find_in_node(N->keys[0], index);
		//cout << "find in parent" << N->keys[0] << " " << index << endl;
		//get N_temp and K_temp(the value between N and N_temp)
		if(index == 0 && find_flag == 0){
			//the left most child
			N_temp =  N->parent->children[1];
			previous_flag = 0;
			K_temp = N->parent->keys[0];
		}
		else{
			//get the right N_temp and K_temp
			if(find_flag == 0) index = index - 1;
			if(index != N->parent->key_count - 1){
				//not the last child, choose the next one
			 	N_temp = N->parent->children[index + 1 + 1];
			 	previous_flag = 0;
			 	K_temp = N->parent->keys[index + 1];
			}
			else{
				//the last child, choose the previous one
				N_temp = N->parent->children[index];
				previous_flag = 1;
				K_temp = N->parent->keys[index];
			}
		}*/
        if((N_temp->key_count + N->key_count < m_degree) && !(N_temp->key_count == m_degree - 1 && N->children[1] == -1)){
            //cout << "begin coalesce nodes! " << endl;
            if(previous_flag == 0){
				//N is previous to N_temp
				
            	Node temp = new TreeNode<keytype>(N->is_leaf, m_degree, key_size, type, file_name, blockf_file_handler_ptr);
				//if the leaf_head points to N, after swap, the block of N and N_temp exchanges
				//the leaf_head_number is N_temp->block, which is right
				//however, need to repoint leaf_head to N_temp
				if(leaf_head_number == N->node_number)
					leaf_head_number == N_temp->node_number;
            	temp->replace(file_name, N->node_number, blockf_file_handler_ptr);
            	N->replace(file_name, N_temp->node_number, blockf_file_handler_ptr);
            	N_temp->replace(file_name, temp->node_number, blockf_file_handler_ptr);
				
				
                //swap the value between N and N_temp
                /*//cout << "begin swap !!!" << endl;
                //cout << "swap N" << endl; N->Print();
                //cout << "swap N_temp" << endl; N->Print();
                int key_count_temp;
                vector<keytype> keys_temp;
                vector<Node> children_temp;
                vector<offset> address_of_tuple_temp;
                //N_temp -> temp
                key_count_temp = N_temp->key_count;
                for (int i = 0; i < N_temp->key_count; ++i)
                {
                    keys_temp.push_back(N_temp->keys[i]);
                    children_temp.push_back(N_temp->children[i]);
                    address_of_tuple_temp.push_back(N_temp->address_of_tuple[i]);
                }
                children_temp.push_back(N_temp->children[N_temp->key_count]);
                //N -> N_temp
                N_temp->key_count = N->key_count;
                for (int i = 0; i < N->key_count; ++i)
                {
                    N_temp->keys[i] = N->keys[i];
                    N_temp->children[i] = N->children[i];
                    N_temp->address_of_tuple[i] = N->address_of_tuple[i];
                }
                N_temp->children[N->key_count] = N->children[N->key_count];
                //temp -> N
                N->key_count = key_count_temp;
                for(int i = 0; i < key_count_temp; i++){
                    N->keys[i] = keys_temp[i];
                    N->children[i] = children_temp[i];
                    N->address_of_tuple[i] = address_of_tuple_temp[i];
                }
                N->children[key_count_temp] = children_temp[key_count_temp];*/
                //finish the swap
            }
            //cout << "N_temp:" << endl; N_temp->Print();
            //cout << "N:" << endl; N->Print();
            if(N->is_leaf != 1){
                N_temp->keys[N_temp->key_count] = K_temp;
                N_temp->key_count++;
                for (int i = 0; i < N->key_count; ++i)
                {
                    N_temp->keys[N_temp->key_count] = N->keys[i];
                    N_temp->children[N_temp->key_count] = N->children[i];
                    N_temp->key_count ++;
                }
                N_temp->children[N_temp->key_count] = N->children[N->key_count];
                //N_temp->Print();
            }
            else{
                for (int i = 0; i < N->key_count; ++i)
                {
                    N_temp->keys[N_temp->key_count] = N->keys[i];
                    N_temp->address_of_tuple[N_temp->key_count] = N->address_of_tuple[i];
                    N_temp->key_count ++;
                }
                N_temp->next_leaf_node = N->next_leaf_node;
            }
            //cout << "N_temp:" << endl; N_temp->Print();
            Node Parent = new TreeNode<keytype>(file_name, N->parent, blockf_file_handler_ptr);
            N_temp->unpin();
            delete_entry(K_temp, Parent, !previous_flag);
            delete N;
            node_count --;
        }
        else{
            //cout << "redsitricbution !" << endl;
            //redsitricbution: borrow an entry form N_temp
            if(previous_flag == 1){
                //N_temp is the previous sibling of N
                if(N->is_leaf != 1){
                    blocknum M =  N_temp->children[N_temp->key_count];
                    keytype K_temp_temp = N_temp->keys[N_temp->key_count - 1];
                    N_temp->keys[N_temp->key_count - 1] = keytype();
                    N_temp->children[N_temp->key_count] = -1;
                    N_temp->key_count --;
                    //move the M and K_temp into N
                    N->children[N->key_count + 1] = N->children[N->key_count];
                    for (int i = 1; i <= N->key_count; ++i)
                    {
                        N->keys[i] = N->keys[i - 1];
                        N->children[i] = N->children[i - 1];
                    }
                    N->keys[0] = K_temp;
                    N->children[0] = M;
                    N->key_count ++;
                    //replace K_temp in parent node by K_temp_temp
                    for (int i = 0; i < Parent->key_count; ++i)
                    {
                        if(Parent->keys[i] == K_temp){
                            Parent->keys[i] = K_temp_temp;
              
                            break;
                        }
                    }
                    //finish redistribution in non-leaf node
                }
                else{
                    offset M_address = N_temp->address_of_tuple[N_temp->key_count - 1];
                    keytype K_temp_temp = N_temp->keys[N_temp->key_count - 1];
                    N_temp->keys[N_temp->key_count - 1] = keytype();
                    N_temp->address_of_tuple[N_temp->key_count - 1] = offset();
                    N_temp->key_count--;
                    //move the M and K_temp_temp into N
                    for (int i = 1; i <= N->key_count; ++i)
                    {
                        N->keys[i] = N->keys[i - 1];
                        N->address_of_tuple[i] = N->address_of_tuple[i - 1];
                    }
                    N->keys[0] = K_temp_temp;
                    N->address_of_tuple[0] = M_address;
                    N->key_count ++;
                    //replace K_temp in parent node by K_temp_temp
                    for (int i = 0; i < Parent->key_count; ++i)
                    {
                        if(Parent->keys[i] == K_temp){
                            Parent->keys[i] = K_temp_temp;
                            break;
                        }
                    }
                    //finish redistribution in leaf node
                }             
            }
            else{
                //symmetric opration, N_temp is the next sibling of N
                if(N->is_leaf != 1){
                    blocknum M = N_temp->children[0];
                    keytype K_temp_temp = N_temp->keys[0];
                    for (int i = 0; i < N_temp->key_count; ++i)
                    {
                    	N_temp->children[i] = N_temp->children[i + 1];
                    }
                    for (int i = 0; i < N_temp->key_count - 1; ++i)
                    {
                    	N_temp->keys[i] = N_temp->keys[i + 1];
                    }
                    N_temp->keys[N_temp->key_count - 1] = keytype();
                    N_temp->children[N_temp->key_count] = -1;
                    N_temp->key_count --;
                    //append M at N
                    N->keys[N->key_count] = K_temp;
                    N->children[N->key_count + 1] = M;
                    N->key_count ++;
                    //replace K_temp in N->parent by K_temp_temp
                    for (int i = 0; i < Parent->key_count; ++i)
                     {
                     	if (Parent->keys[i] == K_temp)
                     	{
                     		Parent->keys[i] = K_temp_temp;
                     		break;
                     	}
                     }
                    //finish redistribution in non-leaf node

                }
                else{
                    offset M_address = N_temp->address_of_tuple[0];
                    keytype K_temp_temp = N_temp->keys[0];
                    for (int i = 0; i < N_temp->key_count - 1; ++i)
                    {
                    	N_temp->keys[i] = N_temp->keys[i + 1];
                    	N_temp->address_of_tuple[i] = N_temp->address_of_tuple[i + 1];
                    }
                    N_temp->keys[N_temp->key_count - 1] = keytype();
                    N_temp->address_of_tuple[N_temp->key_count - 1] = offset();
                    N_temp->key_count --;
                    //append K_temp_temp M_address into N
                    N->keys[N->key_count] = K_temp_temp;
                    N->address_of_tuple[N->key_count] = M_address;
                    N->key_count++;
                    //replace K_temp by K_temp_temp in parent node
                    Node Parent = new TreeNode<keytype>(file_name, N->parent, blockf_file_handler_ptr);
                    for (int i = 0; i < Parent->key_count; ++i)
                    {
                        if(Parent->keys[i] == K_temp){
                            Parent->keys[i] = N_temp->keys[0];
                            break;
                        }
                    }
                    //finish redistribution in leaf node
                }
            }
			Parent->unpin();
        	N_temp->unpin();
			N->unpin(); 
			//finish redistribution
			node_count ++;
        }
		return;
    }
	N->unpin();
	return;
}
//is_empty
//input:void
//output:whether the tree is empty
//bugs:
//
template <class keytype>
bool Bplus_Tree<keytype>::is_empty(){
	return (root_number == -1 || node_count == 0);
}
//for debug
//input:void
//output:the structure of tree
//bugs:
//

template <class keytype>
void Bplus_Tree<keytype>::Print_Tree(){
	
	if(root_number == -1)
		return ;
	//cout << "Print_Tree()" << endl; 
	//cout << "root_number" << root_number << endl;
	Node N = new TreeNode<keytype>(file_name, root_number, blockf_file_handler_ptr);
	if(N->is_leaf){
		Print_Node(N->node_number);
	}
	else{
		for(int i = 0; i <= N->key_count; i++){
			//N->replace(file_name, N->children[i], blockf_file_handler_ptr);
			Print_Node(N->children[i]);
			if(i == N->key_count)
				break;
			//cout << N->keys[i] << endl;
		}	
	}
	N->unpin();
}

template <class keytype>
void Bplus_Tree<keytype>::Print_Node(blocknum number){
	if(number == -1){
		return ;
	}
	else{
		Node N  = new TreeNode<keytype>(file_name, number, blockf_file_handler_ptr);
		if(N->is_leaf){
			for(int i = 0; i < N->key_count; i++){
				//cout << N->keys[i] << "->" << N->address_of_tuple[i] << endl;
			}
		}
		else{
			for(int i = 0; i <= N->key_count; i++){
				Print_Node(N->children[i]);
				if(i == N->key_count)
                    break;
				//cout << N->keys[i] << endl;
			}
		
		}
		N->unpin();
	}
	return ;
}


