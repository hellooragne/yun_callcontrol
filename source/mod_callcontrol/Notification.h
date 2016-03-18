#ifndef MOD_CALLCONTROL_RESPONSEMESSAGEHANDLECENTER_H_
#define MOD_CALLCONTROL_RESPONSEMESSAGEHANDLECENTER_H_

#include <map>
#include <string>

#include "switch.h"
//#include "Common.h"
//#include "DataStruct.h"
#include "Handler.h"
#include "ThreadUtil.h"

/******************************************
 * description: used to handle all response 
 *				mesage and dipatch message
 */
//class CResponseMessageHandleCenter
//{
//public:
//	CResponseMessageHandleCenter();
//	~CResponseMessageHandleCenter() {};
//
//	int SubscribeMessage(const char *sToken, DataPackage_t *pData);
//	int UnsubscribeMessage(const char *sToken);
//	int DispatchMessage(const char *sToken, int iStatusCode, const char *sBody);
//	
//	int Init(switch_memory_pool_t *pMemoryPool);
//
//private:
//	TMutex									m_mutex;
//	std::map<std::string, DataPackage_t *>	m_messageRouteTable;
//};


/******************************************
 * description: used to handle all response 
 *				mesage and dipatch message
 */
class TNotification
{
public:
	TNotification(switch_memory_pool_t *pMemoryPool);
	~TNotification();

	int Subscribe(const char *sId, Handler *handler);
	int Unsubscribe(const char *sId);
	int Dispatch(const char *sId, int iType, const char *sMsg);

private:
	TMutex	m_mutex;
	std::map<std::string, Handler *> m_msgRouteTable;
};


#endif