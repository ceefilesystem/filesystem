#define _CRT_SECURE_NO_WARNINGS
#include "httpImpl.h"
#include "httpParser.h"
#include <sstream>

std::queue<HttpRequest*> HttpParser::reqUploadQueue;
std::queue<HttpRequest*> HttpParser::reqDownloadQueue;
std::queue<HttpRequest*> HttpParser::reqOtherQueue;

HttpResponse::HttpResponse()
{
	httpCode = 200;
	httpPhrase = "OK";

	httpPhrase.clear();
	httpBody.clear();
	httpHeaders.clear();
}

HttpResponse::~HttpResponse()
{
	httpCode = 404;
	httpPhrase = "NOT FOUNT";

	httpPhrase.clear();
	httpBody.clear();
	httpHeaders.clear();
}

void HttpResponse::setResponse(headerMap& headers, std::string &Body)
{
	httpBody = Body;
	httpHeaders = headers;

	return;
}

std::string HttpResponse::getResponse()
{
	std::ostringstream ostream;
	ostream << "HTTP/1.1" << " " << httpCode << " " << httpPhrase << "\r\n"
		<< "Connection: keep-alive" << "\r\n";

	headerMapIter iter = httpHeaders.begin();

	while (iter != httpHeaders.end())
	{
		ostream << iter->first << ": " << iter->second << "\r\n";
		++iter;
	}
	ostream << "Content-Length: " << httpBody.size() << "\r\n\r\n";
	ostream << httpBody;

	return ostream.str();
}

void HttpResponse::resetResponse()
{
	httpCode = 200;
	httpPhrase = "OK";

	httpBody.clear();
	httpHeaders.clear();
}

HttpParser::HttpParser(http_parser *parser, http_parser_settings* parser_settings)
{
	this->parser = parser;
	this->parser->data = this;

	//初始化
	http_parser_init(this->parser, HTTP_REQUEST);
	http_parser_settings_init(parser_settings);

	//设置解析回调
	this->parserSettings = parser_settings;
	this->parserSettings->on_message_begin = on_message_begin_cb;
	this->parserSettings->on_url = on_url_cb;
	this->parserSettings->on_header_field = on_header_field_cb;
	this->parserSettings->on_header_value = on_header_value_cb;
	this->parserSettings->on_headers_complete = on_headers_complete_cb;
	this->parserSettings->on_body = on_body_cb;
	this->parserSettings->on_message_complete = on_message_complete_cb;
}

HttpParser::~HttpParser()
{
}

size_t HttpParser::HttpParseRequest(const char *data, size_t len)
{
	size_t nparsed = http_parser_execute(parser, parserSettings, data, len);
	if (parser->http_errno != HPE_OK)
	{
		return -1;
	}

	return nparsed;
}

int HttpParser::on_message_begin_cb(http_parser *parser)
{
	printf("\n***MESSAGE BEGIN***\n\n");
	HttpParser* hp = (HttpParser*)(parser->data);
	hp->http_request_parser = new HttpRequest();

	return 0;
}

int HttpParser::on_url_cb(http_parser *parser, const char *at, size_t length)
{
	printf("Url: %.*s\n", (int)length, at);

	char urlBuf[256] = { 0 };
	sprintf(urlBuf, "%.*s", (int)length, at);
	_strlwr(urlBuf);

	HttpParser* hp = (HttpParser*)(parser->data);
	hp->http_request_parser->httpUrl.assign(at, length);

	//区分上传下载
	if(strstr(urlBuf, "?upload"))
		reqUploadQueue.push(hp->http_request_parser);
	else if (strstr(urlBuf, "?download"))
		reqDownloadQueue.push(hp->http_request_parser);
	else
		reqOtherQueue.push(hp->http_request_parser);

	return 0;
}

int HttpParser::on_header_field_cb(http_parser *parser, const char *at, size_t length)
{
	printf("Header field: %.*s\n", (int)length, at);
	HttpParser* hp = (HttpParser*)(parser->data);
	hp->http_request_parser->httpHeaderField.assign(at, length);

	return 0;
}

int HttpParser::on_header_value_cb(http_parser *parser, const char *at, size_t length)
{
	printf("Header value: %.*s\n", (int)length, at);
	HttpParser* hp = (HttpParser*)(parser->data);
	HttpRequest *request = hp->http_request_parser;
	request->httpHeaders[request->httpHeaderField] = std::string(at, length);

	return 0;
}

int HttpParser::on_headers_complete_cb(http_parser *parser)
{
	HttpParser* hp = (HttpParser*)(parser->data);
	HttpRequest *request = hp->http_request_parser;
	request->httpMethod = http_method_str((http_method)parser->method);

	return 0;
}

// 可能会多次调用
int HttpParser::on_body_cb(http_parser *parser, const char *at, size_t length)
{
	printf("Body: %.*s\n", (int)length, at);
	HttpParser* hp = (HttpParser*)(parser->data);
	hp->http_request_parser->httpBody.append(at, length);

	return 0;
}

int HttpParser::on_message_complete_cb(http_parser *parser)
{
	printf("\n***MESSAGE COMPLETE***\n\n");
	HttpParser* hp = (HttpParser*)(parser->data);
	HttpRequest *request = hp->http_request_parser;

	for (size_t i = 0; i < reqOtherQueue.size(); i++)	{
		HttpRequest *request = reqOtherQueue.back();
		delete request;
		request = nullptr;
		reqOtherQueue.pop();
	}

	return 0;
}
