#ifndef _MAPPED_FILE_WIN_H_
#define _MAPPED_FILE_WIN_H_
/*
	ӳ���ļ���  ӳ���ļ����ڴ� �ṩ��ȡ����
*/

#include <windows.h>
#include <stdio.h>

class TMappedFile
{
public:
	TMappedFile(const char * filePath);
	~TMappedFile();
private:
	HANDLE _dumpFileDescriptor = INVALID_HANDLE_VALUE;
	HANDLE _fileMappingObject = NULL;
	void * _mappedFileAddress = NULL;
	size_t _mappedFileSize = 0;
public:
	// ��ȡ �������ļ�����
	// pos: ��ʼλ��
	// count: ��ȡ�ֽ���
	// out: ����ָ��
	// ���أ�ʵ�ʶ�ȡ��������  ���󷵻�-1
	int readRang(int pos, int count, void * out);
	// д�� �������ļ�����
	// pos: ��ʼλ��
	// size: д���ֽ���
	// in: ����ָ��
	// ���أ�д��ɹ�0  ���󷵻�-1
	int writeRang(int pos, int size, void * in);
	int mappedFileSize();
};
#endif

