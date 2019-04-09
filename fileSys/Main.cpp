//#include <iostream>
//#include <fstream>
//#include <thread>
//#include <vector>
//#include "mapFile.h"
//#include <Windows.h>
//
//void workR1(const char * FileName, size_t begpos, size_t offset)
//{
//	readByMapFile* rm = new readByMapFile(FileName);
//
//	char * out = nullptr;
//	rm->readDate(0, 1024, &out);
//
//	char tmp[1024 + 1] = { 0 };
//	memcpy(tmp, out, 1024);
//	std::cout << rm->getUseCout() << "----" << tmp << std::endl;
//
//	while (true)
//	{
//		std::cout << rm->getUseCout() << std::endl;
//		Sleep(1000);
//	}
//
//	delete rm;
//	return;
//}
//
//void workR2(const char * FileName, size_t begpos, size_t offset)
//{
//	readByMapFile* rm = new readByMapFile(FileName);
//
//	char * out = nullptr;
//	rm->readDate(1024, 1024, &out);
//
//	char tmp[1024 + 1] = { 0 };
//	memcpy(tmp, out, 1024);
//	std::cout << rm->getUseCout() << "----" << tmp << std::endl;
//
//	delete rm;
//	return;
//}
//
//void workW1(const char * FileName, size_t begpos, size_t offset)
//{
//	writeByMapFile* wm = new writeByMapFile(FileName, 2048);
//
//	char out[] = "abcdefghig";
//	wm->writeDate(0, 10, out);
//
//	while (true)
//	{
//		std::cout << wm->getUseCout() << std::endl;
//		Sleep(1000);
//	}
//
//	delete wm;
//	return;
//}
//
//void workW2(const char * FileName, size_t begpos, size_t offset)
//{
//	writeByMapFile* wm = new writeByMapFile(FileName, 2048);
//
//	char out[] = "1234567890";
//	wm->writeDate(10, 10, out);
//
//	std::cout << wm->getUseCout();
//
//	delete wm;
//	return;
//}
//
//int main(int argc, char *argv[])
//{
//	const char *FileName = "file.txt";
//	size_t FileSize = 2048;
//
//	std::thread t1(workR1, FileName, 0, 0);
//	std::thread t2(workR2, FileName, 0, 0);
//	t1.detach();
//	t2.detach();
//		
//	std::thread t3(workW1, FileName, 0, 0);
//	std::thread t4(workW2, FileName, 0, 0);
//	t3.join();
//	t4.join();
//
//	system("pause");
//	return 0;
//}

#include <iostream>
#include "fileService.h"
#include "timeWheel.h"
#include "httpServer.h"

void fun1(void* data)
{
	std::cout << (char*)data << std::endl;
}

void fun2(void)
{
	std::cout << "aaa" << std::endl;
}

void fun3(void)
{
	std::cout << "bbb" << std::endl;
}

int main(int argc, char *argv[])
{

	httpServer* ht = new httpServer();
	//uvServer* ht = new uvServer();

	ht->start("127.0.0.1", 9999);

	system("pause");
	return 0;
}