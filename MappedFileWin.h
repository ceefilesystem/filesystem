#ifndef _MAPPED_FILE_WIN_H_
#define _MAPPED_FILE_WIN_H_
/*
	ӳ���ļ���  ӳ���ļ����ڴ� �ṩ��ȡ����
*/
/*
test
*/

#include <windows.h>
#include <stdio.h>


extern struct mappedFileWin;

extern int CreateMappedFile(const char * filePath, mappedFileWin * mappedFile);
extern int mappedFileReadRang(mappedFileWin * mappedFile, int pos, int count, void * out);
extern int mappedFileWriteRang(mappedFileWin * mappedFile, int pos, int size, void * in);
extern int CloseMappedFIle(mappedFileWin * mappedFile);




#endif

