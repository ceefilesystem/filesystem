#pragma once
#include <map>
#include <queue>

struct http_parser;
struct http_parser_settings;

class HttpRequest;

typedef std::map<std::string, std::string> headerMap;
typedef headerMap::iterator headerMapIter;
typedef std::queue<HttpRequest*> reqQueue; //��Ϣ����

// ����
class HttpRequest
{
public:
	std::string httpMethod;
	std::string httpUrl;

	headerMap   httpHeaders;
	std::string httpHeaderField;
	std::string httpBody;

	HttpRequest() {};
	~HttpRequest() {};
};

// Ӧ��
class HttpResponse
{
public:
	int         httpCode;
	std::string httpPhrase;
	headerMap   httpHeaders;
	std::string httpBody;

	HttpResponse();
	~HttpResponse();

	std::string getResponse();
	void setResponse(headerMap& headers, std::string &Body);
	void resetResponse();
};

// ����
class HttpParser
{
private:
	http_parser *parser;
	http_parser_settings *parserSettings;

	HttpRequest *http_request_parser;

	//HttpRequest *requestUpload;
	//HttpRequest *requestDownLoad;

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
	
	size_t HttpParseRequest(const char *data, size_t len);

	static std::queue<HttpRequest*> reqUploadQueue; //�ϴ���Ϣ����
	static std::queue<HttpRequest*> reqDownloadQueue; //������Ϣ����
	static std::queue<HttpRequest*> reqOtherQueue; //������Ϣ����
};
