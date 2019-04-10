#include "httpParser.h"
#include "httpServer.h"
#include <iostream>

#define BACKLOG 128

#define RESPONSE                  \
  "HTTP/1.1 200 OK\r\n"           \
  "Content-Type: text/plain\r\n"  \
  "Content-Length: 14\r\n"        \
  "\r\n"                          \
  "Hello, World!\n"

typedef struct client_t {
	uv_tcp_t handle;
	http_parser parser;
	void* arg;
} client_t;

/* tcp callbacks */
static void on_close_cb(uv_handle_t *);
static void on_shutdown_cb(uv_shutdown_t *, int);
static void on_alloc_cb(uv_handle_t*, size_t, uv_buf_t*);
static void on_write_cb(uv_write_t*, int);
static void on_read_cb(uv_stream_t*, ssize_t, const uv_buf_t*);

/* http callbacks */
static int on_message_begin(http_parser*);
static int on_headers_complete(http_parser*);
static int on_message_complete(http_parser*);
static int on_header_field(http_parser*);
static int on_header_value(http_parser*);
static int on_body(http_parser*);
static int on_url(http_parser*);

/* tcp callbacks */
static void on_close_cb(uv_handle_t *handle) {
	client_t *client = (client_t *)handle->data;
	free(client);
}

static void on_shutdown_cb(uv_shutdown_t *shutdown_req, int status) {
	uv_close((uv_handle_t *)shutdown_req->handle, on_close_cb);
	free(shutdown_req);
}

static void on_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
	ASSERT(buf->base != NULL);
}

static void on_write_cb(uv_write_t* write_req, int status) {
	uv_close((uv_handle_t *)write_req->handle, on_close_cb);
	free(write_req);
}

static void on_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
	int r = 0;
	client_t *client = (client_t *)handle->data;

	if (nread < 0) {
		if (nread == UV_EOF) {
			std::cout << "客户端主动断开\n";
		}
		else if (nread == UV_ECONNRESET) {
			std::cout << "客户端异常断开\n";
		}
		else {
			std::cout << "客户端)异常断开\n";
		}

		free(buf->base);
		uv_shutdown_t *shutdown_req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
		r = uv_shutdown(shutdown_req, handle, on_shutdown_cb);
		ASSERT(r == 0);
		return;
	}

	if (nread == 0) {
		free(buf->base);
		return;
	}

	int ret = 0;
	if (nread > 0) {
		//解析请求
		http_parser_settings* parser_settings = ((httpServer*)(client->arg))->getHttpParserSets();
		size_t parsed = http_parser_execute(&client->parser, parser_settings, buf->base, nread);

		if (parsed < nread) {
			printf("parse error\n");
			uv_close((uv_handle_t *)handle, on_close_cb);
		}

		free(buf->base);
		return;
	}
}

/* http callback */
static int on_message_begin(http_parser* _) {
	(void)_;
	printf("\n***MESSAGE BEGIN***\n\n");
	return 0;
}

static int on_message_complete(http_parser* _) {
	(void)_;
	printf("\n***MESSAGE COMPLETE***\n\n");
	return 0;
}

static int on_url(http_parser* _, const char* at, size_t length) {
	(void)_;

	//TODO区分上传下载
	// GET /UpLoad?filename  上传

	// GET /DownLoad?filename 下载


	printf("Url: %.*s\n", (int)length, at);
	return 0;
}

static int on_header_field(http_parser* _, const char* at, size_t length) {
	(void)_;

	//TODO区分上传下载
	printf("Header field: %.*s\n", (int)length, at);
	return 0;
}

static int on_header_value(http_parser* _, const char* at, size_t length) {
	(void)_;
	printf("Header value: %.*s\n", (int)length, at);
	return 0;
}

static int on_body(http_parser* _, const char* at, size_t length) {
	(void)_;

	//上传的 保存到磁盘
	printf("Body: %.*s\n", (int)length, at);
	return 0;
}

static int on_headers_complete(http_parser* parser) {
	client_t *client = (client_t *)parser->data;

	uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
	uv_buf_t buf = uv_buf_init((char*)RESPONSE, sizeof(RESPONSE));
	int r = uv_write(write_req, (uv_stream_t *)&client->handle, &buf, 1, on_write_cb);
	ASSERT(r == 0);

	return 1;
}

httpServer::httpServer()
{
	try
	{
		uv_os_setenv("UV_THREADPOOL_SIZE", "120");
		this->loop = uv_default_loop();
		tcpServer.loop = this->loop;
		tcpServer.data = this;

		this->parser_settings = (http_parser_settings*)malloc(sizeof(struct http_parser_settings));

		//设置解析回调
		parser_settings->on_message_begin = on_message_begin;
		parser_settings->on_url = on_url;
		parser_settings->on_header_field = on_header_field;
		parser_settings->on_header_value = on_header_value;
		parser_settings->on_headers_complete = on_headers_complete;
		parser_settings->on_body = on_body;
		parser_settings->on_message_complete = on_message_complete;
	}
	catch (const std::exception& e)
	{
		throw e;
	}
}

httpServer::~httpServer()
{
	if (this->loop)
		uv_loop_close(loop);

	if (this->parser_settings)
		free(parser_settings);
}

void httpServer::setUpCallBack(upLoadCallBack upCallBack)
{
	this->upCallBack = upCallBack;
}

void httpServer::setDownCallBack(downLoadCallBack downCallBack)
{
	this->downCallBack = downCallBack;
}

upLoadCallBack httpServer::getUpCallBack()
{
	return this->upCallBack;
}

downLoadCallBack httpServer::getDownCallBack()
{
	return this->downCallBack;
}

void httpServer::on_connection_cb(uv_stream_t *server, int status)
{
	if (status != 0) {
		fprintf(stderr, "Connect error %s\n", uv_err_name(status));
	}
	ASSERT(status == 0);

	client_t *client = (client_t *)malloc(sizeof(client_t));
	int r = uv_tcp_init(server->loop, &client->handle);
	ASSERT(r == 0);
	client->handle.data = client;

	r = uv_accept(server, (uv_stream_t *)&client->handle);
	if (r) {
		uv_shutdown_t *shutdown_req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
		uv_shutdown(shutdown_req, (uv_stream_t *)&client->handle, on_shutdown_cb);
		ASSERT(r == 0);
	}

	http_parser_init(&client->parser, HTTP_REQUEST);
	client->parser.data = client;
	client->arg = server->data;
	r = uv_read_start((uv_stream_t *)&client->handle, on_alloc_cb, on_read_cb);
}

http_parser_settings * httpServer::getHttpParserSets()
{
	return this->parser_settings;
}

int httpServer::start(const char * ip, int port)
{
	struct sockaddr_in addr;
	int r;

	ASSERT(0 == uv_ip4_addr(ip, port, &addr));

	r = uv_tcp_init(loop, &tcpServer);
	if (r) {
		fprintf(stderr, "Socket creation error\n");
		return 1;
	}

	r = uv_tcp_bind(&tcpServer, (const struct sockaddr*) &addr, 0);
	if (r) {
		fprintf(stderr, "Bind error\n");
		return 1;
	}

	r = uv_listen((uv_stream_t*)&tcpServer, SOMAXCONN, on_connection_cb);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_err_name(r));
		return 1;
	}

	return 0;
}

void httpServer::run()
{
	uv_run(loop, UV_RUN_DEFAULT);
	return;
}
