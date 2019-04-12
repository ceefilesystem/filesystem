#pragma once
#include "uvTask.h"

typedef int(*downLoadCallBack) (void* info, void** out);
typedef int(*upLoadCallBack) (void* in);

class uvServer
{
	friend static void on_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf);
	friend static void on_connection_cb(uv_stream_t* server, int status);

private:
	upLoadCallBack upCallBack;
	downLoadCallBack downCallBack;

private:
	uv_loop_t* loop;
	uv_tcp_t tcpHandle;

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