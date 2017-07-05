#include "interpreter_internal.h"
#include "catlogmanager.h"
#include <cstdio>

Package *Interpreter::interpreter(const std::string &command, std::string &error)
{
	using namespace std;
	using namespace _Interpreter;
	using namespace CatlogManager;
	vector<Word> words;
	Package *package;
	char errMessage[200];

	if (scan(command, words, error))
	{
		return NULL;
	}
	/*for (it = words.begin(); it<words.end(); it++)
	{
		printf("%d,%d,%d,%d,%d,%lf, %s\n", it->type, it->line, it->relativePos, it->l, it->r, it->value, it->str);
	}*/
	if ((package = getCommand(words, error)) == NULL)
	{
		return NULL;
	}
	
	//exam valid
	char *temp;
	std::vector<CreateTPack::Attri> attris;
	std::vector<CreateTPack::Attri>::iterator ita;
	CreateTPack *ctp;	SelectPack *sp;
	CreateIPack *cip;	InsertPack *ip;
	DropTPack *dtp;		DeletePack *dp;
	DropIPack *dip;
	switch(package->type)
	{
		
		case Package::CREATET_PACK:
			ctp = dynamic_cast<CreateTPack *>(package);
			if (getTable(ctp->table,temp))
			{
				delete[] temp;
				sprintf(errMessage,"table created before\n");//l和r似乎是没有用的
				error = error + errMessage;
				return NULL;
			}
			break;
		case Package::CREATEI_PACK:
			cip = dynamic_cast<CreateIPack *>(package);
			if (getIndex(cip->table, cip->attri,temp) || getIndex(cip->index,temp))
			{
				delete[] temp;
				sprintf(errMessage,"index exist\n");//l和r似乎是没有用的
				error = error + errMessage;
				return NULL;
			}
			break;
		case Package::DROPT_PACK:
			dtp = dynamic_cast<DropTPack *>(package);
			if (!getTable(dtp->table,temp))
			{
				sprintf(errMessage,"table not exist\n");//l和r似乎是没有用的
				error = error + errMessage;
				return NULL;
			}
			delete[] temp;
			break;
		case Package::DROPI_PACK:
			dip = dynamic_cast<DropIPack *>(package);
			if (!getIndex(dip->index,temp))
			{
				sprintf(errMessage,"index not exist\n");//l和r似乎是没有用的
				error = error + errMessage;
				return NULL;
			}
			delete[] temp;
			break;
		case Package::SELECT_PACK:
			sp = dynamic_cast<SelectPack *>(package);
			if (!getTable(sp->table,temp))
			{
				sprintf(errMessage,"table not exist\n");//l和r似乎是没有用的
				error = error + errMessage;
				return NULL;
			}
			delete[] temp;
			attris.clear();
			getAttribute(sp->table, attris);
			//printf("aaaa");
			//printf("%s", sp->attri[0]);
			for (std::vector<char*>::iterator it = sp->attri.begin(); it != sp->attri.end(); it++)
			{
				//printf("%s", *it);
				if (!strcmp(*it, "*")) continue;
				for (ita = attris.begin(); ita != attris.end(); ita++)
				{
					if (!strcmp(ita->name, *it)) break;
				}
				if (ita == attris.end())
				{
					sprintf(errMessage,"%s attribute not exist\n", *it);//l和r似乎是没有用的
					error = error + errMessage;
					return NULL;
				}
			}
			for (std::vector<SelectPack::Condition>::iterator it = sp->condition.begin(); it != sp->condition.end(); it++)
			{
				for (ita = attris.begin(); ita != attris.end(); ita++)
				{
					if (!strcmp(ita->name, it->attri))
					{
						if (ita->type != it->type && !(ita->type == Package::FLOAT_ && it->type == Package::INT_))//not consistent
						{
							sprintf(errMessage,"%s's type not consistent\n", it->attri);//l和r似乎是没有用的
							error = error + errMessage;
							return NULL;
						}
						break;
					}
				}
				if (ita == attris.end())//not exist
				{
					sprintf(errMessage,"%s attribute not exist\n", it->attri);//l和r似乎是没有用的
					error = error + errMessage;
					return NULL;
				}
			}
			break;
		case Package::INSERT_PACK:
			ip = dynamic_cast<InsertPack *>(package);
			if (!getTable(ip->table,temp))
			{
				sprintf(errMessage,"table not exist\n");//l和r似乎是没有用的
				error = error + errMessage;
				return NULL;
			}
			delete[] temp;
			getAttribute(ip->table,attris);
			for (int i = 0; i<(int)attris.size(); i++)
			{
				if (i >= (int)ip->value.size() || 
					(ip->value[i].type != attris[i].type 
						&& !(ip->value[i].type == Package::INT_ && attris[i].type == Package::FLOAT_)))
				{
					sprintf(errMessage,"value not consistent\n");//l和r似乎是没有用的
					error = error + errMessage;
					return NULL;
				}
			}
			break;
		case Package::DELETE_PACK:
			dp = dynamic_cast<DeletePack *>(package);
			if (!getTable(dp->table,temp))
			{
				sprintf(errMessage,"table not exist\n");//l和r似乎是没有用的
				error = error + errMessage;
				return NULL;
			}
			delete[] temp;
			getAttribute(dp->table,attris);
			for (std::vector<DeletePack::Condition>::iterator it = dp->condition.begin(); it != dp->condition.end(); it++)
			{
				for (ita = attris.begin(); ita != attris.end(); ita++)
				{
					if (!strcmp(ita->name, it->attri))
					{
						if (ita->type != it->type && !(ita->type == Package::FLOAT_ && it->type == Package::INT_))//not consistent
						{
							sprintf(errMessage,"%s's type not consistent\n", it->attri);//l和r似乎是没有用的
							error = error + errMessage;
							return NULL;
						}
						break;
					}
				}
				if (ita == attris.end())//not exist
				{
					sprintf(errMessage,"%s attribute not exist", it->attri);//l和r似乎是没有用的
					error = error + errMessage;
					return NULL;
				}
			}
			break;
		case Package::EXECFILE_PACK:
			break;
		case Package::QUIT_PACK:
			break;
	}
	/*CreateTPack *ctp = dynamic_cast<CreateTPack *>(package);
	printf("%s\n", ctp->table);
	for (vector<Attri>::iterator it = ctp->attri.begin(); it!=ctp->attri.end(); it++)
	{
		printf("%s %d %d %d\n", it->name, it->type, it->size, it->unique);
	}
	printf("%s\n", ctp->PK);*/
	/*CreateIPack *cip = dynamic_cast<CreateIPack *>(package);
	printf("%s %s %s\n", cip->index, cip->table, cip->attri);*/
	/*DropIPack *dip = dynamic_cast<DropIPack *>(package);
	printf("%s", dip->index);*/
	/*DropTPack *dtp = dynamic_cast<DropTPack *>(package);
	printf("%s", dtp->table);*/
	/*SelectPack *sp = dynamic_cast<SelectPack *>(package);
	printf("%s\n",sp->table);
	for (vector<Condition>::iterator it = sp->condition.begin(); it != sp->condition.end(); it++)
	{
		if (it->type == INT_)
		printf("%s %d %d\n", it->attri, it->op, it->value.i);
		else if (it->type == FLOAT_)
		printf("%s %d %lf\n", it->attri, it->op, it->value.f);
		else if (it->type == CHAR_)
		printf("%s %d %s\n", it->attri, it->op, it->value.c);
	}*/
	/*InsertPack *ip = dynamic_cast<InsertPack *>(package);
	printf("%s\n", ip->table);
	for (vector<Value>::iterator it = ip->value.begin(); it != ip->value.end(); it++)
	{
		if (it->type == INT_)
		printf("%d\n", it->value.i);
		else if (it ->type == FLOAT_)
		printf("%lf\n", it->value.f);
		else if(it->type == CHAR_)
		printf("%s\n", it->value.c);
	}*/
	/*DeletePack *sp = dynamic_cast<DeletePack *>(package);
	printf("%s\n",sp->table);
	for (vector<Condition>::iterator it = sp->condition.begin(); it != sp->condition.end(); it++)
	{
		if (it->type == INT_)
		printf("%s %d %d\n", it->attri, it->op, it->value.i);
		else if (it->type == FLOAT_)
		printf("%s %d %lf\n", it->attri, it->op, it->value.f);
		else if (it->type == CHAR_)
		printf("%s %d %s\n", it->attri, it->op, it->value.c);
	}*/
	/*ExePack *ep = dynamic_cast<ExePack *>(package);
	printf("%s\n", ep->file);*/

	return package;
}



