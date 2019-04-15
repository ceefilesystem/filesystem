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

#define RX_BUFFER_SIZE 1024 * 1024  //���ջ�������С
#define TX_BUFFER_SIZE 1024 * 1024  //���ͻ�������С

// ��Ϣ�ṹ
struct msg {
	void *payload;
	size_t len;
	char binary;
	char first;
	char final;
};

// �ͻ��˵Ľṹ��
struct per_session_data {
	/*��һ���ͻ��˽��*/
	struct per_session_data *pss_list;
	/*�ͻ������Ӿ��*/
	struct lws *wsi;
	/*��ǰ���յ�����Ϣ���*/
	int last;
};

//�������ṹ��
struct per_vhost_data {
	struct lws_context *context;
	/*������������vhost��protocol��ȡ�ýṹ��*/
	struct lws_vhost *vhost;
	/*ʹ�õ�Э��*/
	const struct lws_protocols *protocol;
	/*�ͻ�������*/
	struct per_session_data *pss_list;
	/*���յ�����Ϣ�������СΪһ������*/
	struct msg amsg;
	/*��ǰ��Ϣ��ţ�����ͬ�����пͻ��˵���Ϣ*/
	int current;
};

//callback
static void destroy_message_cb(void *);
static int lws_function_cb(struct lws *,
	enum lws_callback_reasons reason, void *, void *, size_t);

//libwebsocketЭ��
//struct lws_protocols {
//	/*Э������*/
//	const char *name;
//	/*����ص���Э���¼�����*/
//	lws_callback_function *callback;
//	/*�������ͶϿ�ʱ�����ڴ��С��Ҳ��callback��user���ڴ�*/
//	size_t per_session_data_size;
//	/*���ջ�������С*/
//	size_t rx_buffer_size;
//	/*Э��id��������������Э��*/
//	unsigned int id;
//	/*�Զ�������*/
//	void *user;
//	/*���ͻ����С��Ϊ0����rx_buffer_size��ͬ*/
//	size_t tx_packet_size;
//};

static struct lws_protocols protocols[] = {
	{"websocket1-server", lws_function_cb, sizeof(struct per_session_data), RX_BUFFER_SIZE, 0, NULL, 0 },
	{ NULL, NULL, 0, 0 } /* terminator */
};

/*��Ϣ�ͷŻص�*/
static void destroy_message_cb(void *_msg)
{
	struct msg *msg = (struct msg *)_msg;
	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}

/*����ص�����*/
static int lws_function_cb(struct lws *wsi,
	enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	/*��ȡ�ͻ��˽ṹ*/
	struct per_session_data *pss = (struct per_session_data *)user;
	/*��vhost��protocol��ԭlws_protocol_vh_priv_zalloc����Ľṹ*/
	struct per_vhost_data *vhd = (struct per_vhost_data *)
		lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

	switch (reason)
	{
		int m;

		/*��ʼ�� */
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
		/*�������� */
		case LWS_CALLBACK_ESTABLISHED:
		{
			lwsl_user("LWS_CALLBACK_ESTABLISHED\n");
			pss->pss_list = vhd->pss_list;
			vhd->pss_list = pss;
			pss->wsi = wsi;
			pss->last = vhd->current;

			break;
		}
		/*�ͻ��˿�д */
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
		/*�յ����� */
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

			//�������еĿͻ��ˣ������ݷ���д��ص�
			lws_start_foreach_llp(struct per_session_data **,
				ppss, vhd->pss_list) {
				lws_callback_on_writable((*ppss)->wsi);
			} lws_end_foreach_llp(ppss, pss_list);
			break;
		}
		/*���ӹر� 4*/
		case LWS_CALLBACK_CLOSED:
		{
			lwsl_user("LWS_CALLBACK_CLOSED\n");
			//�����ͻ�������
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