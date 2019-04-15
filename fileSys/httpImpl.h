#pragma once
#include <map>
#include <queue>

struct http_parser;
struct http_parser_settings;

typedef std::map<std::string, std::string> headerMap;
typedef headerMap::iterator headerMapIter;

typedef std::queue<HttpRequest*> reqQueue; //消息队列

typedef struct Connection {
	HttpRequest  *http_request_parser;    //解析时用
	HttpRequest  *http_request_process;   //处理请求时用
	HttpResponse  http_response;
	HttpParser    http_parser;
	std::queue<HttpRequest*> req_queue;
}Connection;

class HttpRequest
{
	friend class HttpParser;

public:
	std::string http_method;
	std::string http_url;

	headerMap   http_headers;
	std::string http_header_field;
	std::string http_body;

	HttpRequest() {};
	~HttpRequest() {};
};

class HttpResponse
{
	friend class HttpParser;

private:
	int         http_code;
	std::string http_phrase;
	headerMap   http_headers;
	std::string http_body;

	HttpResponse();
	~HttpResponse();

	std::string getResponse();
	void setResponse();
	void resetResponse();
};

class HttpParser
{
private:
	http_parser *parser;
	http_parser_settings* parser_settings;

public:
	HttpParser(http_parser *parser, http_parser_settings* parser_settings);
	~HttpParser();
	
	int HttpParseRequest(const std::string &inbuf);

	static int OnMessageBeginCallback(http_parser *parser);
	static int OnUrlCallback(http_parser *parser, const char *at, size_t length);
	static int OnHeaderFieldCallback(http_parser *parser, const char *at, size_t length);
	static int OnHeaderValueCallback(http_parser *parser, const char *at, size_t length);
	static int OnHeadersCompleteCallback(http_parser *parser);
	static int OnBodyCallback(http_parser *parser, const char *at, size_t length);
	static int OnMessageCompleteCallback(http_parser *parser);
};