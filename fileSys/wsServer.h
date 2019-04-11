#pragma once
#define LWS_PLUGIN_STATIC
#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#pragma comment(lib, "websockets.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#else
#include <libwebsockets.h>
#pragma comment(lib, "websockets_static.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#endif

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
};

