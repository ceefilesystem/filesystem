#pragma once
#include <libwebsockets.h>

typedef int(*downLoadCallBack) (void* in, void** out);
typedef int(*upLoadCallBack) (void* in);

class wsServer
{
private:
	upLoadCallBack upCallBack;
	downLoadCallBack downCallBack;

private:
	struct lws_context * context;
	struct lws_context_creation_info info;

public:
	wsServer();
	~wsServer();

	void setUpCallBack(upLoadCallBack upCallBack);
	void setDownCallBack(downLoadCallBack downCallBack);

	upLoadCallBack getUpCallBack();
	downLoadCallBack getDownCallBack();

	int start(const char* ip, int port);
	void run();

protected:
	void setProtocols(struct lws_protocols* protocols);
};

