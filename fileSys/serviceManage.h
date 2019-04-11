#pragma once

typedef enum ProtocolType
{
	TCP = 0,
	HTTP = 1,
	WEBSOCKET = 2
}ProtocolType;

void* initService(ProtocolType type);
void startService(void* handle);
void stopService(void* handle);
void canceService(void* handle);
void deleteService(void* handle);