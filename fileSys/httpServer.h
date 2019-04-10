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

	static void on_connection_cb(uv_stream_t *, int);

public:
	httpServer();
	~httpServer();

	void setUpCallBack(upLoadCallBack upCallBack);
	void setDownCallBack(downLoadCallBack downCallBack);

	upLoadCallBack getUpCallBack();
	downLoadCallBack getDownCallBack();

	http_parser_settings* getHttpParserSets();
	int start(const char* ip, int port);
	void run();
};
