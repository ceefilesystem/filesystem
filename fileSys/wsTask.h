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

#define RX_BUFFER_SIZE 1024 * 1024  //接收缓存区大小
#define TX_BUFFER_SIZE 1024 * 1024  //发送缓存区大小

// 消息结构
struct msg {
	void *payload;
	size_t len;
	char binary;
	char first;
	char final;
};

// 客户端的结构体
struct per_session_data {
	/*下一个客户端结点*/
	struct per_session_data *pss_list;
	/*客户端连接句柄*/
	struct lws *wsi;
	/*当前接收到的消息编号*/
	int last;
};

//服务器结构体
struct per_vhost_data {
	struct lws_context *context;
	/*服务器，可由vhost与protocol获取该结构体*/
	struct lws_vhost *vhost;
	/*使用的协议*/
	const struct lws_protocols *protocol;
	/*客户端链表*/
	struct per_session_data *pss_list;
	/*接收到的消息，缓存大小为一条数据*/
	struct msg amsg;
	/*当前消息编号，用来同步所有客户端的消息*/
	int current;
};

//callback
static void destroy_message_cb(void *);
static int lws_function_cb(struct lws *,
	enum lws_callback_reasons reason, void *, void *, size_t);

//libwebsocket协议
//struct lws_protocols {
//	/*协议名称*/
//	const char *name;
//	/*服务回调，协议事件处理*/
//	lws_callback_function *callback;
//	/*服务建立和断开时申请内存大小，也是callback中user的内存*/
//	size_t per_session_data_size;
//	/*接收缓存区大小*/
//	size_t rx_buffer_size;
//	/*协议id，可以用来区分协议*/
//	unsigned int id;
//	/*自定义数据*/
//	void *user;
//	/*发送缓存大小，为0则与rx_buffer_size相同*/
//	size_t tx_packet_size;
//};

static struct lws_protocols protocols[] = {
	{"websocket1-server", lws_function_cb, sizeof(struct per_session_data), RX_BUFFER_SIZE, 0, NULL, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

/*消息释放回调*/
static void destroy_message_cb(void *_msg)
{
	struct msg *msg = (struct msg *)_msg;
	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}

/*服务回调函数*/
static int lws_function_cb(struct lws *wsi,
	enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	/*获取客户端结构*/
	struct per_session_data *pss = (struct per_session_data *)user;
	/*由vhost与protocol还原lws_protocol_vh_priv_zalloc申请的结构*/
	struct per_vhost_data *vhd = (struct per_vhost_data *)
		lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

	switch (reason)
	{
		int m;

		/*初始化 */
		case LWS_CALLBACK_PROTOCOL_INIT:
		{
			vhd = (struct per_vhost_data *)lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
				lws_get_protocol(wsi), sizeof(struct per_vhost_data));
			if (!vhd)
				return -1;

			vhd->protocol = (struct lws_protocols *)lws_get_protocol(wsi);
			vhd->context = lws_get_context(wsi);
			vhd->vhost = lws_get_vhost(wsi);

			break;
		}
		/*建立连接 */
		case LWS_CALLBACK_ESTABLISHED:
		{
			lwsl_user("LWS_CALLBACK_ESTABLISHED\n");
			pss->pss_list = vhd->pss_list;
			vhd->pss_list = pss;
			pss->wsi = wsi;
			pss->last = vhd->current;

			break;
		}
		/*客户端可写 */
		case LWS_CALLBACK_SERVER_WRITEABLE:
		{
			if (!vhd->amsg.payload)
				break;

			if (pss->last == vhd->current)
				break;
			
			lwsl_user("LWS_CALLBACK_SERVER_WRITEABLE\n");

			/* notice we allowed for LWS_PRE in the payload already */
			m = lws_write(wsi, (unsigned char *)vhd->amsg.payload + LWS_PRE, vhd->amsg.len,
				LWS_WRITE_TEXT);
			if (m < vhd->amsg.len) {
				//lwsl_err("ERROR %d writing to di socket\n", n);
				return -1;
			}

			pss->last = vhd->current;
			break;
		}
		/*收到数据 */
		case LWS_CALLBACK_RECEIVE:
		{
			if (vhd->amsg.payload)
				destroy_message_cb(&vhd->amsg);

			lwsl_user("LWS_CALLBACK_RECEIVE\n");
			vhd->amsg.len = len;

			/* notice we over-allocate by LWS_PRE */
			vhd->amsg.payload = malloc(LWS_PRE + len);
			if (!vhd->amsg.payload) {
				lwsl_user("OOM: dropping\n");
				break;
			}

			lwsl_user("%s\n", in);
			memcpy((char *)vhd->amsg.payload + LWS_PRE, in, len);
			vhd->current++;

			//遍历所有的客户端，将数据放入写入回调
			lws_start_foreach_llp(struct per_session_data **,
				ppss, vhd->pss_list) {
				lws_callback_on_writable((*ppss)->wsi);
			} lws_end_foreach_llp(ppss, pss_list);
			break;
		}
		/*连接关闭 4*/
		case LWS_CALLBACK_CLOSED:
		{
			lwsl_user("LWS_CALLBACK_CLOSED\n");
			//遍历客户端链表
			lws_start_foreach_llp(struct per_session_data **,
				ppss, vhd->pss_list) {
			if (*ppss == pss) {
				*ppss = pss->pss_list;
				break;
			}
			} lws_end_foreach_llp(ppss, pss_list);
			break;
		}
		default:
			break;
	}

	return 0;
}