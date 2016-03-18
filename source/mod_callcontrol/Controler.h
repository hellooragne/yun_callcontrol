#ifndef MOD_CALLCONTROL_CONTROLER_H_
#define MOD_CALLCONTROL_CONTROLER_H_

#include "switch.h"
//#include "Common.h"
//#include "ResponseMessageHandleCenter.h"
//#include "ThreadSync.h"

#include "Config.h"
#include "ThreadUtil.h"
#include "Notification.h"

/**********************************
 * description: control the global operation
 */

class TManager
{
public:
	TManager(switch_memory_pool_t *pMemoryPool);
	~TManager();

	//about cmd
	void ExecuteCmd(const char *cmd, switch_stream_handle_t *stream);

	//about notification
	void HandleEvent(const char *sId, int iType, const char *sMsg);
	int Subscribe(const char *sId, Handler *handler);
	int Unsubscribe(const char *sId);

	//about config
	void LoadConfig(const char *sFileName);
	char* GetProfileName();
	char* GetEventString(); 
	char* GetContentType();
	char* GetFromUser();
	int GetTimeOut();

	TThreadCond *GetThreadCond();
	void ReleaseThreadCond(TThreadCond *threadCond);

	void Work();
	void Stop();
	void Shutdown();
	
private:
	bool m_isRunning;
	int  m_iThreadAmount;
	switch_memory_pool_t *m_pMemoryPool;
	TMutex m_mutex;
	TConfig	m_config;
	TThreadCondPool m_condPool;
	TNotification m_notification;
};

#endif