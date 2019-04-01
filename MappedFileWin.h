#ifndef _MAPPED_FILE_WIN_H_
#define _MAPPED_FILE_WIN_H_
/*
	ӳ���ļ���  ӳ���ļ����ڴ� �ṩ��ȡ����
*/

void* createMappedFile(const char * filePath);
int mappedFileReadRang(void * mappedFile, int pos, int count, void * out);
int mappedFileWriteRang(void * mappedFile, int pos, int size, void * in);
void closeMappedFile(void * mappedFile);

size_t getFileSize(void * mappedFile);
const char* getFilePath(void * mappedFile);

#endif

