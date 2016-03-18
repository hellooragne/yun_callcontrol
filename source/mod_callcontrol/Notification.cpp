#include "Notification.h"


/*********************** TNotification ****************************/
TNotification :: TNotification(switch_memory_pool_t *pMemoryPool) 
	: m_mutex(pMemoryPool)
{
}

TNotification :: ~TNotification() 
{	
}

int TNotification :: Subscribe(const char *sId, Handler *pHandler)
{
	if (sId == NULL || pHandler == NULL)
	{
		return SWITCH_STATUS_FALSE;
	}

	m_mutex.Lock();
	std::string strId(sId);
	m_msgRouteTable.insert(std::pair<std::string, Handler *>(strId, pHandler));
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "add notification map item: %s\n", sId);
	m_mutex.UnLock();

	return SWITCH_STATUS_SUCCESS;
}

int TNotification :: Unsubscribe(const char *sId)
{
	if (sId == NULL)
	{
		return SWITCH_STATUS_FALSE;
	}

	m_mutex.Lock();
	std::map<std::string, Handler *>::iterator itr;
	std::string strId(sId);
	itr = m_msgRouteTable.find(strId);
	if (itr != m_msgRouteTable.end())
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "delete notification map item: %s\n", sId);
		m_msgRouteTable.erase(itr);
	}
	m_mutex.UnLock();

	return SWITCH_STATUS_SUCCESS;
}

int TNotification :: Dispatch(const char *sId, int iType, const char *sMsg)
{
	if (sId == NULL)
	{
		return SWITCH_STATUS_FALSE;
	}

	m_mutex.Lock();
	std::map<std::string, Handler *>::iterator itr;
	std::string strId(sId);
	itr = m_msgRouteTable.find(strId);
	if (itr != m_msgRouteTable.end())
	{
		itr->second->Handle(iType, sMsg);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "dispatch notification event: %s\n", sId);
	}
	m_mutex.UnLock();

	return SWITCH_STATUS_SUCCESS;
}

/********************** end of TNotification **********************/ 