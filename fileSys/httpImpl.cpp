#define _CRT_SECURE_NO_WARNINGS
#include "httpImpl.h"
#include "httpParser.h"
#include <sstream>

httpRequest::httpRequest() {
	IsUpLoad = false;
	IsDownLoad = false;
};

httpRequest::~httpRequest() {
	IsUpLoad = false;
	IsDownLoad = false;
	//httpHeaders.clear();
	//httpUrl.clear();
	//httpBody.clear();
};

httpResponse::httpResponse()
{
	httpPhrase.clear();
	httpBody.clear();
	httpHeaders.clear();

	httpCode = 200;
	httpPhrase = "OK";
}

httpResponse::~httpResponse()
{
	httpPhrase.clear();
	httpBody.clear();
	httpHeaders.clear();

	httpCode = 404;
	httpPhrase = "NOT FOUNT";
}

std::string httpResponse::getResponse()
{
	std::ostringstream ostream;
	ostream << "HTTP/1.1" << " " << httpCode << " " << httpPhrase << "\r\n";
		//<< "Connection: keep-alive" << "\r\n";

	headerMapIter iter = httpHeaders.begin();

	while (iter != httpHeaders.end())
	{
		ostream << iter->first << ": " << iter->second << "\r\n";
		++iter;
	}

	ostream << "Content-Length: " << httpBody.size()<< "\r\n\r\n";
	ostream << httpBody;

	return ostream.str();
}

void httpResponse::resetResponse()
{
	httpCode = 200;
	httpPhrase = "OK";

	httpBody.clear();
	httpHeaders.clear();
}

httpParser::httpParser(http_parser *parser, http_parser_settings* parser_settings)
{
	this->parser = parser;
	this->parser->data = this;
	this->parserSettings = parser_settings;
	this->request = nullptr;
}

httpParser::~httpParser()
{
	if (this->request) {
		delete this->request;
		this->request = nullptr;
	}

	if (this->parser) {
		free(this->parser);
		this->parser = nullptr;
	}
}

size_t httpParser::httpParseRequest(const char *data, size_t len)
{
	size_t nparsed = http_parser_execute(parser, parserSettings, data, len);
	if (parser->http_errno != HPE_OK)
	{
		return -1;
	}

	return nparsed;
}

httpRequest * httpParser::getRequest()
{
	if (this->request) {
		return this->request;
	}

	return nullptr;
}

int httpParser::on_message_begin_cb(http_parser *parser)
{
	//printf("\n***MESSAGE BEGIN***\n\n");
	httpParser* hp = (httpParser*)(parser->data);
	hp->request = new httpRequest();

	return 0;
}

int httpParser::on_url_cb(http_parser *parser, const char *at, size_t length)
{
	//printf("Url: %.*s\n", (int)length, at);

	char urlBuf[256] = { 0 };
	sprintf(urlBuf, "%.*s", (int)length, at);
	_strlwr(urlBuf);

	httpParser* hp = (httpParser*)(parser->data);
	hp->request->httpUrl.assign(at, length);

	//区分上传下载
	if (strstr(urlBuf, "?upload")) {
		hp->request->IsUpLoad = true;
	}
	else if (strstr(urlBuf, "?download")) {
		hp->request->IsDownLoad = true;
	}

	return 0;
}

int httpParser::on_header_field_cb(http_parser *parser, const char *at, size_t length)
{
	//printf("Header field: %.*s\n", (int)length, at);
	httpParser* hp = (httpParser*)(parser->data);
	hp->request->httpHeaderField.assign(at, length);

	return 0;
}

int httpParser::on_header_value_cb(http_parser *parser, const char *at, size_t length)
{
	//printf("Header value: %.*s\n", (int)length, at);
	httpParser* hp = (httpParser*)(parser->data);
	httpRequest *request = hp->request;
	request->httpHeaders[request->httpHeaderField] = std::string(at, length);

	return 0;
}

int httpParser::on_headers_complete_cb(http_parser *parser)
{
	httpParser* hp = (httpParser*)(parser->data);
	httpRequest *request = hp->request;
	request->httpMethod = http_method_str((http_method)parser->method);

	return 0;
}

// 可能会多次调用
int httpParser::on_body_cb(http_parser *parser, const char *at, size_t length)
{
	//printf("Body: %.*s\n", (int)length, at);
	httpParser* hp = (httpParser*)(parser->data);
	hp->request->httpBody.append(at, length);

	return 0;
}

int httpParser::on_message_complete_cb(http_parser *parser)
{
	//printf("\n***MESSAGE COMPLETE***\n\n");
	httpParser* hp = (httpParser*)(parser->data);

	return 0;
}
