#include "wsServer.h"
#include "ws.h"

wsServer::wsServer()
{
}

wsServer::~wsServer()
{
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
	struct lws_context_creation_info info;
	struct lws_context *context;
	const char *p;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
		/* for LLL_ verbosity above NOTICE to be built into lws,
		 * lws must have been configured and built with
		 * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
		 /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
		 /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
		/* | LLL_DEBUG */;

	signal(SIGINT, sigint_handler);

	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws client echo + permessage-deflate + multifragment bulk message\n");
	lwsl_user("   lws-minimal-ws-client-echo [-n (no exts)] [-p port] [-o (once)]\n");


	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.port = port;
	info.protocols = protocols;
	info.pvo = &pvo;
	info.vhost_name = ip;
	//if (!lws_cmdline_option(argc, argv, "-n"))
	//	info.extensions = extensions;
	info.pt_serv_buf_size = 32 * 1024;
	info.options = LWS_SERVER_OPTION_VALIDATE_UTF8 |
		LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return 1;
	}

	while (n >= 0 && !interrupted)
		n = lws_service(context, 1000);

	lws_context_destroy(context);

	lwsl_user("Completed %s\n", interrupted == 2 ? "OK" : "failed");

	return interrupted != 2;
}

void wsServer::run()
{
}
