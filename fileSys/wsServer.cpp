#include "wsServer.h"
#include "wsTask.h"

wsServer::wsServer()
{
	this->context = nullptr;
	memset(&this->info, 0, sizeof(this->info));

	//设置日志等级
	int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
	lws_set_log_level(logs, NULL);
}

wsServer::~wsServer()
{
	lws_context_destroy(context);
}

void wsServer::setUpCallBack(upLoadCallBack upCallBack)
{
}

void wsServer::setDownCallBack(downLoadCallBack downCallBack)
{
}

upLoadCallBack wsServer::getUpCallBack()
{
	return upLoadCallBack();
}

downLoadCallBack wsServer::getDownCallBack()
{
	return downLoadCallBack();
}

int wsServer::start(const char * ip, int port)
{
	info.port = port;
	info.protocols = protocols;
	info.pvo = &pvo;
	info.vhost_name = ip;
	info.pt_serv_buf_size = 32 * 1024;
	info.options = LWS_SERVER_OPTION_VALIDATE_UTF8 |
		LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE |
		LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;
	
	context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return 1;
	}

	return 0;
}

void wsServer::run()
{
	int n = 0;
	while (n >= 0 && !interrupted)
		n = lws_service(context, 1000);

	lwsl_user("Completed %s\n", interrupted == 2 ? "OK" : "failed");

	return;
}
