#pragma once
#define LWS_PLUGIN_STATIC
#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#else
#include <libwebsockets.h>
#endif

#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "websockets_static.lib")

typedef int(*downLoadCallBack) (void* in, void** out);
typedef int(*upLoadCallBack) (void* in);

class wsServer
{
private:
	struct lws_context * context;
	struct lws_context_creation_info info;


	upLoadCallBack upCallBack;
	downLoadCallBack downCallBack;

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

