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
} client_t;

static void close_cb(uv_handle_t *);
static void shutdown_cb(uv_shutdown_t *, int);
static void alloc_cb(uv_handle_t*, size_t, uv_buf_t*);
static void write_cb(uv_write_t*, int);
static int headers_complete_cb(http_parser*);

/* tcp callbacks */
void close_cb(uv_handle_t *handle) {
	client_t *client = (client_t *)handle->data;
	free(client);
}

void shutdown_cb(uv_shutdown_t *shutdown_req, int status) {
	uv_close((uv_handle_t *)shutdown_req->handle, close_cb);
	free(shutdown_req);
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
	ASSERT(buf->base != NULL);
}

void write_cb(uv_write_t* write_req, int status) {
	uv_close((uv_handle_t *)write_req->handle, close_cb);
	free(write_req);
}

/* http callback */
int headers_complete_cb(http_parser* parser) {
	client_t *client = (client_t *)parser->data;

	uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
	uv_buf_t buf = uv_buf_init((char*)RESPONSE, sizeof(RESPONSE));
	int r = uv_write(write_req, (uv_stream_t *)&client->handle, &buf, 1, write_cb);
	ASSERT(r == 0);

	return 1;
}

httpServer::httpServer()
{
	uv_os_setenv("UV_THREADPOOL_SIZE", "120");
	this->loop = uv_default_loop();
	this->parser_settings = (http_parser_settings*)malloc(sizeof(struct http_parser_settings));
	parser_settings->on_headers_complete = headers_complete_cb;
}


httpServer::~httpServer()
{
	if (this->loop)
		uv_loop_close(loop);

	if (this->parser_settings)
		free(parser_settings);
}


void httpServer::read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
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
		r = uv_shutdown(shutdown_req, handle, shutdown_cb);
		ASSERT(r == 0);
		return;
	}

	if (nread == 0) {
		free(buf->base);
		return;
	}

	int ret = 0;
	if (nread > 0) {
		http_parser_settings* parser_settings = ((httpServer*)(client->parser.data))->parser_settings;
		size_t parsed = http_parser_execute(&client->parser, parser_settings, buf->base, nread);

		if (parsed < nread) {
			printf("parse error\n");
			uv_close((uv_handle_t *)handle, close_cb);
		}
	}
}

void httpServer::connection_cb(uv_stream_t *server, int status)
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
		uv_shutdown(shutdown_req, (uv_stream_t *)&client->handle, shutdown_cb);
		ASSERT(r == 0);
	}

	http_parser_init(&client->parser, HTTP_REQUEST);
	client->parser.data = server->data;
	r = uv_read_start((uv_stream_t *)&client->handle, alloc_cb, read_cb);
}

int httpServer::start(const char * ip, int port)
{
	struct sockaddr_in addr;
	int r = uv_ip4_addr(ip, port, &addr);
	ASSERT(r == 0);

	r = uv_tcp_bind(&tcpServer, (struct sockaddr *) &addr, 0);
	ASSERT(r == 0);

	tcpServer.loop = this->loop;
	tcpServer.data = this;
	r = uv_listen((uv_stream_t *)&tcpServer, BACKLOG, connection_cb);

	return 0;
}

void httpServer::run()
{
	uv_run(loop, UV_RUN_DEFAULT);
	return;
}
