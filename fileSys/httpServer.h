#pragma once
#include "uvTask.h"

class httpParser;
struct http_parser_settings;
typedef int(*downLoadCallBack) (void* in, void** out);
typedef int(*upLoadCallBack) (void* in);

class httpServer
{
	friend static void on_read_cb(uv_stream_t* ,ssize_t ,const uv_buf_t*);
	friend static void on_queuework_cb(uv_work_t* uw);
	friend static void on_connection_cb(uv_stream_t *, int);

private:
	upLoadCallBack upCallBack;
	downLoadCallBack downCallBack;

private:
	uv_loop_t* loop;
	uv_tcp_t tcpServer;

	http_parser_settings* parser_settings;

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