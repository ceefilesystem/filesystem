#ifndef _MAPPED_FILE_WIN_H_
#define _MAPPED_FILE_WIN_H_
/*
	映射文件类  映射文件到内存 提供读取操作
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
	// 读取 区域内文件数据
	// pos: 开始位置
	// count: 读取字节数
	// out: 数据指针
	// 返回：实际读取数据数量  错误返回-1
	int readRang(int pos, int count, void * out);
	// 写入 区域内文件数据
	// pos: 开始位置
	// size: 写入字节数
	// in: 数据指针
	// 返回：写入成功0  错误返回-1
	int writeRang(int pos, int size, void * in);
	int mappedFileSize();
};
#endif

