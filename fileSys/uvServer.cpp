#include "uvServer.h"
#include <iostream>

/* tcp callbacks */
static void on_close_cb(uv_handle_t *);
static void on_shutdown_cb(uv_shutdown_t *, int);
static void on_alloc_cb(uv_handle_t*, size_t, uv_buf_t*);
static void on_write_cb(uv_write_t*, int);
static void on_read_cb(uv_stream_t*, ssize_t, const uv_buf_t*);
static void on_queuework_cb(uv_work_t*);
static void on_complete_cb(uv_work_t*, int);
static void on_connection_cb(uv_stream_t* server, int status);

static void on_close_cb(uv_handle_t* peer) 
{
	if (peer)
		free(peer);
}

static void on_shutdown_cb(uv_shutdown_t* req, int status)
{
	if (req->handle) {
		uv_close((uv_handle_t*)req->handle, on_close_cb);
		free(req);
	}
}

static void on_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

static void on_write_cb(uv_write_t* req, int status)
{
	free(req);

	if (status == 0)
		return;

	fprintf(stderr, "uv_write error: %s - %s\n",
		uv_err_name(status), uv_strerror(status));
}

typedef struct  ReadData {
	int nread;
	uv_stream_t* handle;
	char*basebuf;
}ReadData;

static void on_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
	uv_shutdown_t* sreq;

	if (nread < 0) {
		if (nread == UV_EOF) {
			std::cout << "客户端主动断开\n";
		}
		else if (nread == UV_ECONNRESET) {
			std::cout << "客户端异常断开\n";
		}
		else {
			std::cout << "客户端异常断开\n";
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
	
	if (nread > 0) { //加入任务队列

		ReadData* readData = (ReadData*)calloc(1, sizeof(ReadData));
		readData->basebuf = (char*)calloc(1, nread);
		readData->nread = nread;
		readData->handle = handle;
		memcpy(readData->basebuf, buf->base, nread);
		
		uv_work_t* uw = (uv_work_t*)malloc(sizeof(uv_work_t));
		uw->data = readData;
		uv_queue_work(((uvServer*)handle->data)->loop, uw, on_queuework_cb, on_complete_cb);
	} 

	free(buf->base);
	return;
}

static void on_queuework_cb(uv_work_t* uw)
{
	static std::string moreData = "";//粘包数据

	ReadData* readData = (ReadData*)uw->data;

	printf("%s\n", readData->basebuf);
	//解析数据
	uv_write_t* wreq;
	uv_buf_t buf;

	wreq = (uv_write_t*)malloc(sizeof *wreq);
	ASSERT(wreq != NULL);
	buf = uv_buf_init((char*)readData->basebuf, readData->nread);

	if (uv_write(wreq, readData->handle, &buf, 1, on_write_cb)) {
		FATAL("uv_write failed");
	}


	//TODO
	//if (buf->base[0] == 1) {//如果是下载协议号
	//	char* outbuf = nullptr;

	//	downLoadCallBack downCallBack = ((uvServer * )handle->data)->getDownCallBack();
	//	ret = downCallBack((void*)buf->base, (void**)&outbuf);
	//	if (ret != 0) {
			//uv_write_t* wreq;
			//uv_buf_t buf;

			//wreq = (uv_write_t*)malloc(sizeof *wreq);
			//ASSERT(wreq != NULL);
	//		buf = uv_buf_init((char*)outbuf, ret);

	//		if (uv_write(wreq, handle, &buf, 1, on_write_cb)) {
	//			FATAL("uv_write failed");
	//		}
	//	}
	//}
	//else if (buf->base[0] == 2) {//如果是上传歇一会
	//	upLoadCallBack upCallBack = ((uvServer *)handle->data)->getUpCallBack();
	//	ret = upCallBack((void*)buf->base);
	//}
	
	free(readData->basebuf);
	free(readData);
	return;
}

static void on_complete_cb(uv_work_t* uw, int status)
{
	free(uw);
	uw = nullptr;

	if (status)
		throw std::exception(uv_strerror(status));

	return;
}

static void on_connection_cb(uv_stream_t* server, int status)
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
