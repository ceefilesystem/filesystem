#pragma once
#include <map>
#include <queue>

struct http_parser;
struct http_parser_settings;

class HttpRequest;

typedef std::map<std::string, std::string> headerMap;
typedef headerMap::iterator headerMapIter;
typedef std::queue<HttpRequest*> reqQueue; //消息队列

// 请求
class HttpRequest
{
	friend class HttpParser;

private:
	std::string httpHeaderField;

public:
	bool IsUpLoad;
	bool IsDownLoad;
	std::string httpUrl;
	std::string httpMethod;
	headerMap   httpHeaders;
	std::string httpBody;

	HttpRequest() {
		IsUpLoad = false;
		IsDownLoad = false;
	};

	~HttpRequest() {
	
	};
};

// 应答
class HttpResponse
{
public:
	int         httpCode;
	std::string httpPhrase;
	headerMap   httpHeaders;
	std::string httpBody;
	int bodySize;

	HttpResponse();
	~HttpResponse();

	std::string getResponse();
	void resetResponse();
};

// 解析
class HttpParser
{
private:
	http_parser *parser;
	http_parser_settings *parserSettings;

	HttpRequest *request;

	/* http callbacks */
	static int on_message_begin_cb(http_parser *parser);
	static int on_url_cb(http_parser *parser, const char *at, size_t length);
	static int on_header_field_cb(http_parser *parser, const char *at, size_t length);
	static int on_header_value_cb(http_parser *parser, const char *at, size_t length);
	static int on_headers_complete_cb(http_parser *parser);
	static int on_body_cb(http_parser *parser, const char *at, size_t length);
	static int on_message_complete_cb(http_parser *parser);

public:
	HttpParser(http_parser *parser, http_parser_settings* parserSettings);
	~HttpParser();
	
	size_t httpParseRequest(const char *data, size_t len);

	HttpRequest* getRequest();
};
