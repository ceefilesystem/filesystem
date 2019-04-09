#pragma once
#include "uvTask.h"
#include "httpParser.h"

class httpServer
{
private:
	uv_loop_t* loop;
	uv_tcp_t tcpServer;
	http_parser_settings* parser_settings;

	static void connection_cb(uv_stream_t *, int);
	static void read_cb(uv_stream_t*, ssize_t, const uv_buf_t*);

public:
	httpServer();
	~httpServer();

	int start(const char* ip, int port);
	void run();
};

