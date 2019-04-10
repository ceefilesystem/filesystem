#pragma once
#include "uvTask.h"

typedef int(*downLoadCallBack) (void* info, void** out);
typedef int(*upLoadCallBack) (void* in);

class uvServer
{
private:
	uv_loop_t* loop;
	uv_tcp_t tcpHandle;

	upLoadCallBack upCallBack;
	downLoadCallBack downCallBack;

	static void on_connection_cb(uv_stream_t* server, int status);
public:
	uvServer();
	~uvServer();

	void setUpCallBack(upLoadCallBack upCallBack);
	void setDownCallBack(downLoadCallBack downCallBack);

	upLoadCallBack getUpCallBack();
	downLoadCallBack getDownCallBack();

	int start(const char* ip, int port);
	void run();
};