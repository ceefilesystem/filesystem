#pragma once

struct file_update_info
{
	char md5[32];
	int totalSize;
	int fileNameSize;
	char* fileName;
};

struct file_download_info
{
	int fileNameSize;
	char* fileName;
};

// md5   ---  filePath
struct file_data
{
	char  md5[32];
	int posSize;
	int rangSize;
	void * data;
};

typedef struct downLoadInof {
	const char* filename;
	int pos;
	int count;
}downLoadInof;

typedef struct upLoadInof {
	const char* filename;
	int pos;
	int count;
	int totalSize;
}upLoadInof;