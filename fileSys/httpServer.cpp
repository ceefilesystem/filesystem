#include "httpParser.h"
#include "httpServer.h"
#include "httpImpl.h"
#include <iostream>
#include <sstream>

/* tcp callbacks */
static void on_close_cb(uv_handle_t *);
static void on_shutdown_cb(uv_shutdown_t *, int);
static void on_alloc_cb(uv_handle_t*, size_t, uv_buf_t*);
static void on_write_cb(uv_write_t*, int);
static void on_read_cb(uv_stream_t*, ssize_t, const uv_buf_t*);
static void on_queuework_cb(uv_work_t*);
static void on_complete_cb(uv_work_t*, int);
static void on_connection_cb(uv_stream_t *, int);

/* tcp callbacks */
static void on_close_cb(uv_handle_t *handle)
{
	free(handle);
	handle = nullptr;
}

static void on_shutdown_cb(uv_shutdown_t *shutdown_req, int status)
{
	uv_close((uv_handle_t *)shutdown_req->handle, on_close_cb);
	free(shutdown_req);
}

static void on_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) 
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
	ASSERT(buf->base != NULL);
}

static void on_write_cb(uv_write_t* write_req, int status)
{
	free(write_req);
	write_req = nullptr;

	if (status == 0)
		return;

	fprintf(stdout, "uv_write error: %s - %s\n",
		uv_err_name(status), uv_strerror(status));
}

typedef struct httpClient_t {
	uv_tcp_t* tcpHandle;
	httpParser* httpPar;
	httpServer* httpSer;
}httpClient;

static void on_queuework_cb(uv_work_t* uw)
{
	httpClient* httpCli = (httpClient*)uw->data;
	uv_tcp_t* handle = httpCli->tcpHandle;
	httpParser* httpPar = httpCli->httpPar;
	httpServer* httpSer = httpCli->httpSer;

	httpRequest * request = httpPar->getRequest();

	//上传
	if (request->IsUpLoad) {
		upLoadCallBack upCallBack = httpSer->getUpCallBack();
		int ret = upCallBack((void*)request);
		if (ret != 0) {
			//组装应答包
			httpResponse respones;
			respones.httpBody = "UpLoad OK";

			headerMap headers;
			headers["response-content-type"] = "text/plain";
			respones.httpHeaders = headers;

			uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));

			std::string responesBuf = respones.getResponse();
			int len = responesBuf.length();
			uv_buf_t buf = uv_buf_init((char*)responesBuf.c_str(), len);
			int r = uv_write(write_req, (uv_stream_t *)(handle), &buf, 1, on_write_cb);
			ASSERT(r == 0);
		}
	}
	//下载
	else if (request->IsDownLoad) {
		char* outbuf = nullptr;
		downLoadCallBack downCallBack = httpSer->getDownCallBack();
		int ret = downCallBack((void*)request, (void**)&outbuf);
		if (ret != 0) {
			//组装应答包
			httpResponse respones;
			respones.httpBody = std::string(outbuf, ret);
			free(outbuf);

			/*headerMap headers;
			headers["response-content-type"] = "text/plain";
			respones.httpHeaders = headers;*/

			uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));

			std::string responesBuf = respones.getResponse();
			int len = responesBuf.size();
			uv_buf_t buf = uv_buf_init((char*)responesBuf.c_str(), len);
			int r = uv_write(write_req, (uv_stream_t *)(handle), &buf, 1, on_write_cb);
			if (r)
				printf(uv_strerror(r));
			ASSERT(r == 0);
		}
	}

	delete httpPar;
	httpPar = nullptr;
	delete httpCli;
	httpCli = nullptr;

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

static void on_read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
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
		int r = uv_shutdown(shutdown_req, handle, on_shutdown_cb);
		ASSERT(r == 0);
		return;
	}

	if (nread == 0) {
		free(buf->base);
		return;
	}

	if (nread > 0) {
		httpServer* httpSer = (httpServer*)(handle->data);
		//handle->data = nullptr;

		//创建解析器
		http_parser* parser = (http_parser*)malloc(sizeof(http_parser));
		http_parser_init(parser, HTTP_REQUEST);
		//初始化
		httpParser*  httpPar = new httpParser(parser, httpSer->parser_settings);

		//解析请求
		size_t parsed = httpPar->httpParseRequest(buf->base, nread);
		if (parsed < nread) {
			free(parser);
			delete httpPar;
			httpPar = nullptr;

			uv_close((uv_handle_t *)handle, on_close_cb);
		}
		else { //加入任务队列
			httpClient* httpCli = (httpClient*)calloc(1, sizeof(httpClient));
			httpCli->tcpHandle = (uv_tcp_t*)handle;
			httpCli->httpSer = httpSer;
			httpCli->httpPar = httpPar;

			uv_work_t* uw = (uv_work_t*)malloc(sizeof(uv_work_t));
			uw->data = httpCli;
			uv_queue_work(handle->loop, uw, on_queuework_cb, on_complete_cb);
		}
	}

	free(buf->base);
	return;
}

static void on_connection_cb(uv_stream_t *server, int status)
{
	if (status != 0) {
		fprintf(stderr, "Connect error %s\n", uv_err_name(status));
	}

	uv_tcp_t* handle = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	int r = uv_tcp_init(server->loop, handle);
	ASSERT(r == 0);

	r = uv_accept(server, (uv_stream_t *)handle);
	if (r) {
		uv_shutdown_t *shutdown_req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
		uv_shutdown(shutdown_req, (uv_stream_t *)handle, on_shutdown_cb);
	}

	handle->loop = server->loop;
	handle->data = server->data;
	r = uv_read_start((uv_stream_t *)handle, on_alloc_cb, on_read_cb);
	ASSERT(r == 0);
}

httpServer::httpServer()
{
	//uv_os_setenv("UV_THREADPOOL_SIZE", "120");
	this->loop = uv_default_loop();
	tcpServer.loop = this->loop;
	tcpServer.data = this;

	this->parser_settings = (http_parser_settings*)malloc(sizeof(struct http_parser_settings));
	http_parser_settings_init(parser_settings);
	//设置解析回调
	this->parser_settings = parser_settings;
	this->parser_settings->on_message_begin = httpParser::on_message_begin_cb;
	this->parser_settings->on_url = httpParser::on_url_cb;
	this->parser_settings->on_header_field = httpParser::on_header_field_cb;
	this->parser_settings->on_header_value = httpParser::on_header_value_cb;
	this->parser_settings->on_headers_complete = httpParser::on_headers_complete_cb;
	this->parser_settings->on_body = httpParser::on_body_cb;
	this->parser_settings->on_message_complete = httpParser::on_message_complete_cb;
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
