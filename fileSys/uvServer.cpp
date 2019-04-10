#include "uvServer.h"
#include <iostream>

typedef struct write_req_t {
	uv_write_t req;
	uv_buf_t buf;
} write_req_t;

/* tcp callbacks */
static void on_close_cb(uv_handle_t *);
static void on_shutdown_cb(uv_shutdown_t *, int);
static void on_alloc_cb(uv_handle_t*, size_t, uv_buf_t*);
static void on_write_cb(uv_write_t*, int);
static void on_read_cb(uv_stream_t*, ssize_t, const uv_buf_t*);

static void on_close_cb(uv_handle_t* peer) {
	free(peer);
}

static void on_shutdown_cb(uv_shutdown_t* req, int status)
{
	uv_close((uv_handle_t*)req->handle, on_close_cb);
	free(req);
}

static void on_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

static void on_write_cb(uv_write_t* req, int status)
{
	write_req_t* wr;

	/* Free the read/write buffer and the request */
	wr = (write_req_t*)req;
	free(wr->buf.base);
	free(wr);

	if (status == 0)
		return;

	fprintf(stderr, "uv_write error: %s - %s\n",
		uv_err_name(status), uv_strerror(status));
}

static void on_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
	int i;
	write_req_t *wr;
	uv_shutdown_t* sreq;

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
		sreq = (uv_shutdown_t*)malloc(sizeof* sreq);
		ASSERT(0 == uv_shutdown(sreq, handle, on_shutdown_cb));
		return;
	}

	if (nread == 0) {
		free(buf->base);
		return;
	}
	
	int ret = 0;
	if (nread > 0) {
		if (buf->base[0] == 1) {//如果是下载协议号
			char* outbuf = nullptr;

			downLoadCallBack downCallBack = ((uvServer * )handle->data)->getDownCallBack();
			ret = downCallBack((void*)buf->base, (void**)&outbuf);
			if (ret != 0) {
				wr = (write_req_t*)malloc(sizeof *wr);
				ASSERT(wr != NULL);
				wr->buf = uv_buf_init((char*)outbuf, ret);

				if (uv_write(&wr->req, handle, &wr->buf, 1, on_write_cb)) {
					FATAL("uv_write failed");
				}
			}
		}
		else if (buf->base[0] == 2) {//如果是上传歇一会
			upLoadCallBack upCallBack = ((uvServer *)handle->data)->getUpCallBack();
			ret = upCallBack((void*)buf->base);
		}
	}

	free(buf->base);

	return;
}

void uvServer::on_connection_cb(uv_stream_t* server, int status)
{
	uv_stream_t* stream;
	int r;

	if (status != 0) {
		fprintf(stderr, "Connect error %s\n", uv_err_name(status));
	}
	ASSERT(status == 0);

	stream = (uv_stream_t*)malloc(sizeof(uv_tcp_t));
	ASSERT(stream != NULL);
	r = uv_tcp_init(server->loop, (uv_tcp_t*)stream);
	ASSERT(r == 0);

	/* associate server with stream */
	stream->data = server->data;

	r = uv_accept(server, stream);
	ASSERT(r == 0);

	r = uv_read_start(stream, on_alloc_cb, on_read_cb);
	ASSERT(r == 0);
}

uvServer::uvServer()
{
	uv_os_setenv("UV_THREADPOOL_SIZE", "120");
	this->loop = uv_default_loop();
	tcpHandle.loop = this->loop;
	tcpHandle.data = this;
}

uvServer::~uvServer()
{
	if (this->loop)
		uv_loop_close(loop);
}

void uvServer::setUpCallBack(upLoadCallBack upCallBack)
{
	this->upCallBack = upCallBack;
}

void uvServer::setDownCallBack(downLoadCallBack downCallBack)
{
	this->downCallBack = downCallBack;
}

upLoadCallBack uvServer::getUpCallBack()
{
	return this->upCallBack;
}

downLoadCallBack uvServer::getDownCallBack()
{
	return this->downCallBack;
}

int uvServer::start(const char* ip, int port) 
{
	struct sockaddr_in addr;
	int r;

	ASSERT(0 == uv_ip4_addr(ip, port, &addr));

	r = uv_tcp_init(loop, &tcpHandle);
	if (r) {
		fprintf(stderr, "Socket creation error\n");
		return 1;
	}

	r = uv_tcp_bind(&tcpHandle, (const struct sockaddr*) &addr, 0);
	if (r) {
		fprintf(stderr, "Bind error\n");
		return 1;
	}

	r = uv_listen((uv_stream_t*)&tcpHandle, SOMAXCONN, on_connection_cb);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_err_name(r));
		return 1;
	}
	
	return 0;
}

void uvServer::run()
{
	uv_run(loop, UV_RUN_DEFAULT);
	return ;
}
