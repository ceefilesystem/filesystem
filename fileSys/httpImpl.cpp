#include "httpImpl.h"
#include "httpParser.h"
#include <sstream>

HttpResponse::HttpResponse()
{
	http_code = 200;
	http_phrase = "OK";
}

HttpResponse::~HttpResponse()
{
	http_code = 404;
	http_phrase = "NOT FOUNT";

	http_phrase.clear();
	http_body.clear();
	http_headers.clear();
}

void HttpResponse::setResponse()
{
	return;
}

std::string HttpResponse::getResponse()
{
	std::ostringstream ostream;
	ostream << "HTTP/1.1" << " " << http_code << " " << http_phrase << "\r\n"
		<< "Connection: keep-alive" << "\r\n";

	headerMapIter iter = http_headers.begin();

	while (iter != http_headers.end())
	{
		ostream << iter->first << ": " << iter->second << "\r\n";
		++iter;
	}
	ostream << "Content-Length: " << http_body.size() << "\r\n\r\n";
	ostream << http_body;

	return ostream.str();
}

void HttpResponse::resetResponse()
{
	http_code = 200;
	http_phrase = "OK";

	http_body.clear();
	http_headers.clear();
}

HttpParser::HttpParser(http_parser *parser, http_parser_settings* parser_settings)
{
	this->parser = (http_parser*)malloc(sizeof(struct http_parser));
	http_parser_init(parser, HTTP_REQUEST);

	this->parser_settings = (http_parser_settings*)malloc(sizeof(struct http_parser_settings));

	//设置解析回调
	parser_settings->on_message_begin = OnMessageBeginCallback;
	parser_settings->on_url = OnUrlCallback;
	parser_settings->on_header_field = OnHeaderFieldCallback;
	parser_settings->on_header_value = OnHeaderValueCallback;
	parser_settings->on_headers_complete = OnHeadersCompleteCallback;
	parser_settings->on_body = OnBodyCallback;
	parser_settings->on_message_complete = OnMessageCompleteCallback;
}

HttpParser::~HttpParser()
{
	if (this->parser)
		free(this->parser);

	if (this->parser_settings)
		free(this->parser_settings);
}

int HttpParser::HttpParseRequest(const std::string & inbuf)
{
	int nparsed = http_parser_execute(parser, parser_settings, inbuf.c_str(), inbuf.size());

	if (parser->http_errno != HPE_OK)
	{
		return -1;
	}

	return nparsed;
}

/* 开始解析 */
int HttpParser::OnMessageBeginCallback(http_parser *parser)
{
	Connection *con = (Connection*)parser->data;
	con->http_request_parser = new HttpRequest();

	return 0;
}

/* 将解析好的url赋值给http_url */
int HttpParser::OnUrlCallback(http_parser *parser, const char *at, size_t length)
{
	Connection *con = (Connection*)parser->data;
	con->http_request_parser->http_url.assign(at, length);

	return 0;
}

/* 将解析到的header_field暂存在http_header_field中 */
int HttpParser::OnHeaderFieldCallback(http_parser *parser, const char *at, size_t length)
{
	Connection *con = (Connection*)parser->data;
	con->http_request_parser->http_header_field.assign(at, length);

	return 0;
}

/* 将解析到的header_value跟header_field一一对应 */
int HttpParser::OnHeaderValueCallback(http_parser *parser, const char *at, size_t length)
{
	Connection      *con = (Connection*)parser->data;
	HttpRequest *request = con->http_request_parser;
	request->http_headers[request->http_header_field] = std::string(at, length);

	return 0;
}

/* 参照官方文档 */
int HttpParser::OnHeadersCompleteCallback(http_parser *parser)
{
	Connection *con = (Connection*)parser->data;
	HttpRequest *request = con->http_request_parser;
	request->http_method = http_method_str((http_method)parser->method);
	return 0;
}

/* 本函数可能被调用不止一次，因此使用append */
int HttpParser::OnBodyCallback(http_parser *parser, const char *at, size_t length)
{
	Connection *con = (Connection*)parser->data;
	con->http_request_parser->http_body.append(at, length);
	return 0;
}

/* 将解析完毕的消息放到消息队列中 */
int HttpParser::OnMessageCompleteCallback(http_parser *parser)
{
	Connection *con = (Connection*)parser->data;
	HttpRequest *request = con->http_request_parser;

	con->req_queue.push(request);

	delete con->http_request_parser;
	con->http_request_parser = NULL;
	return 0;
}
