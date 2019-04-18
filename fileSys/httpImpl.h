#pragma once
#include <map>
#include <queue>

struct http_parser;
struct http_parser_settings;

class httpRequest;

typedef std::map<std::string, std::string> headerMap;
typedef headerMap::iterator headerMapIter;
typedef std::queue<httpRequest*> reqQueue; //消息队列

// 请求
class httpRequest
{
	friend class httpParser;

private:
	std::string httpHeaderField;

public:
	bool IsUpLoad;
	bool IsDownLoad;
	std::string httpUrl;
	std::string httpMethod;
	headerMap   httpHeaders;
	std::string httpBody;

	httpRequest();
	virtual ~httpRequest();
};

// 应答
class httpResponse
{
public:
	int         httpCode;
	std::string httpPhrase;
	headerMap   httpHeaders;
	std::string httpBody;

	httpResponse();
	virtual ~httpResponse();

	std::string getResponse();
	void resetResponse();
};

// 解析
class httpParser
{
	friend class httpServer;

private:
	http_parser *parser;
	http_parser_settings *parserSettings;

	httpRequest *request;

	/* http callbacks */
	static int on_message_begin_cb(http_parser *parser);
	static int on_url_cb(http_parser *parser, const char *at, size_t length);
	static int on_header_field_cb(http_parser *parser, const char *at, size_t length);
	static int on_header_value_cb(http_parser *parser, const char *at, size_t length);
	static int on_headers_complete_cb(http_parser *parser);
	static int on_body_cb(http_parser *parser, const char *at, size_t length);
	static int on_message_complete_cb(http_parser *parser);

public:
	httpParser(http_parser *parser, http_parser_settings* parserSettings);
	virtual ~httpParser();
	
	size_t httpParseRequest(const char *data, size_t len);

	httpRequest* getRequest();
};
