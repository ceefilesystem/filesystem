#include <iostream>
#include "serviceManage.h"

int main(int argc, char *argv[])
{
	void * handle = initService(ProtocolType::TCP);
	startService(handle);

	system("pause");
	return 0;
}