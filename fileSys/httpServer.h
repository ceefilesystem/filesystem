#pragma once
#include "uvTask.h"

struct http_parser_settings;

typedef int(*downLoadCallBack) (void* in, void** out);
typedef int(*upLoadCallBack) (void* in);

class httpServer
{
private:
	uv_loop_t* loop;
	uv_tcp_t tcpServer;
	http_parser_settings* parser_settings;

	upLoadCallBack upCallBack;
	downLoadCallBack downCallBack;

	static void connection_cb(uv_stream_t *, int);
	static void read_cb(uv_stream_t*, ssize_t, const uv_buf_t*);

public:
	httpServer();
	~httpServer();

	void setUpCallBack(upLoadCallBack upCallBack);
	void setDownCallBack(downLoadCallBack downCallBack);

	upLoadCallBack getUpCallBack();
	downLoadCallBack getDownCallBack();

	int start(const char* ip, int port);
	void run();
};
