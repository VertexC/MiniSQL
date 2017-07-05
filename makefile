ENV = LINUX
FLAG = DEBUG

ifeq (${ENV},LINUX)
	CLEANER = rm
else
	CLEANER = del
endif

#make FLAG = DEBUG
ifeq ($(FLAG),DEBUG)
	CFLAG = -g -Wall -std=gnu++11
	CLEAN = 
else
	CFLAG = -O2 -DDEBUG -std=gnu++11
	CLEAN = ${MAKER} clean
endif

COMPILER = g++
MAKER = mingw32-make

OBJ = BF_BufferManager.o BF_FileHandle.o BFManager.o RM_Filehandle.o RM_Filescan.o rm_manager.o RM_Record.o RM_RID.o

INCLUDE = 

OPATH = .
LIBPATH = .

O = -c $< -o $(OPATH)/$@
DLL = -shared $^ -o $(DLLPATH)/$@
LIB = ar crv ${LIBPATH}/$@ $^ 


all:
	@echo off
	${MAKER} depend
	${MAKER} RMmanager.lib
	${CLEAN}
	@echo on
	@echo finish
include depend

RMmanager.lib:${OBJ}
	${LIB}

%.o:%.cpp
	${COMPILER} ${CFLAG} ${INCLUDE} ${O}

depend:${OBJ:.o=.cpp}
	${COMPILER} ${CFLAG} ${INCLUDE} -MM ${OBJ:.o=.cpp} ${INC} >depend

clean:
	${CLEANER} depend
	${CLEANER} *.o