#include "interpreter.h"
#include "API.h"
#include "catlogmanager.h"
#include <cstdio>
#include <string>
#include <ctime>
using namespace std;
using namespace Interpreter;

int main(){
	string s;
	char temp[1000];
	string error;
	Package *p = NULL;
	FILE* fp = stdin;
	char ch;
	API api;
	clock_t begin, end;
	
	CatlogManager::init();
	while(1)
	{
		s.clear();
		error.clear();
		if (fp == stdin)
		{
			printf(">");
			while(s.find(';')==string::npos)
			{
				ch = fgetc(fp);
				s.append(1,ch);
			}
			ch = fgetc(fp);//\n
		}
		else
		{
			while(!feof(fp) && s.find(';')==string::npos)
			{
				s.append(1,ch);
				if (feof(fp)) break;
				//cout<<ch;
				ch = fgetc(fp);
			}
			while(!feof(fp) && isspace(ch)) ch = fgetc(fp);
			if (feof(fp))
			{
				fclose(fp);
				fp = stdin;
			} 
		}
		begin = clock();
		if (!s.empty() &&!(p = interpreter(s,error)))
		{
			cerr<<error;
		}
		else
		{
			if (p->type == Package::QUIT_PACK) break;
			if (p->type == Package::EXECFILE_PACK)
			{
				ExePack *e = dynamic_cast<ExePack *>(p);
				if (!(fp = fopen(e->file,"r")))
				{
					printf("can't open file\n");
					fp = stdin;
					ch = fgetc(fp);
				}
			}
			else
			{
				api.package_resolve(p);
				delete p;
			}
		}
		end = clock();
		printf("finished in %.3lf time\n\n", ((double)end-begin)/CLOCKS_PER_SEC);
	}
	return 1;
}

