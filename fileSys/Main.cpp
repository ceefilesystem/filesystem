#include <iostream>
#include "serviceManage.h"
#include <boost/beast/websocket.hpp>

int main(int argc, char *argv[])
{
	void * handle = initService(ProtocolType::TCP);
	startService(handle);

	system("pause");
	return 0;
}