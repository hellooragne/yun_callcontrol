#include "ThreadUtil.h"


/*******************************  TMutex  ****************************/
TMutex::TMutex(switch_memory_pool_t *pMemoryPool) : m_pSwitchMutex(NULL)
{
	switch_mutex_init(&m_pSwitchMutex, SWITCH_MUTEX_NESTED, pMemoryPool);
}

TMutex::~TMutex()
{
	switch_mutex_destroy(m_pSwitchMutex);
}
//----------------------------------------------------------

//switch_status_t TMutex::Init(switch_memory_pool_t *pMemoryPool)
//{
//	if (pMemoryPool == NULL) 
//	{
//		return SWITCH_STATUS_FALSE;
//	}
//
//	return switch_mutex_init(&m_pSwitchMutex, SWITCH_MUTEX_NESTED, pMemoryPool);
//}
//----------------------------------------------------------

switch_status_t TMutex::Lock()
{
	//if (m_pSwitchMutex == NULL)
	//{
	//	return SWITCH_STATUS_FALSE;
	//}

	return switch_mutex_lock(m_pSwitchMutex);
}
//----------------------------------------------------------

switch_status_t TMutex::UnLock()
{
	//if (m_pSwitchMutex == NULL)
	//{
	//	return SWITCH_STATUS_FALSE;
	//}

	return switch_mutex_unlock(m_pSwitchMutex);
}
//----------------------------------------------------------

//void TMutex::Release()
//{
//	if (m_pSwitchMutex != NULL)
//	{
//		switch_mutex_destroy(m_pSwitchMutex);
//		m_pSwitchMutex = NULL;
//	}
//}
/************************ end of TMutex ***********************/


/*****************************  TThreadCond  ********************/
TThreadCond::TThreadCond(switch_memory_pool_t *pMemoryPool)
	: m_pMutex(NULL)
	, m_pCond(NULL)
{
	switch_mutex_init(&m_pMutex,SWITCH_MUTEX_NESTED, pMemoryPool);
	switch_thread_cond_create(&m_pCond, pMemoryPool);
}
//----------------------------------------------------------

TThreadCond::~TThreadCond()
{
	/*Release();*/
	switch_mutex_destroy(m_pMutex);
	switch_thread_cond_destroy(m_pCond);

}
//----------------------------------------------------------

//switch_status_t TThreadCond::Init(switch_memory_pool_t *pMemoryPool)
//{
//	if (pMemoryPool == NULL)
//	{
//		return SWITCH_STATUS_FALSE;
//	}
//
//	switch_status_t result;
//	if ((result = switch_mutex_init(&m_pMutex,SWITCH_MUTEX_NESTED, pMemoryPool)) != SWITCH_STATUS_SUCCESS)
//	{
//		return result;
//	}
//
//	if ((result = switch_thread_cond_create(&m_pCond, pMemoryPool)) != SWITCH_STATUS_SUCCESS)
//	{
//		switch_mutex_destroy(m_pMutex);
//	}
//	
//	return result;
//}
//----------------------------------------------------------

switch_status_t TThreadCond::Wait(int iTimeout)
{
	//if (m_pMutex == NULL || m_pCond == NULL)
	//{
	//	return SWITCH_STATUS_FALSE;
	//}

	switch_status_t result;

	if (iTimeout < 0)
	{
		switch_mutex_lock(m_pMutex);
		result = switch_thread_cond_wait(m_pCond, m_pMutex);
		switch_mutex_unlock(m_pMutex);		
	}
	else
	{
		switch_mutex_lock(m_pMutex);
		result = switch_thread_cond_timedwait(m_pCond, m_pMutex, iTimeout);
		switch_mutex_unlock(m_pMutex);
	}

	return result;
}
//----------------------------------------------------------

switch_status_t TThreadCond::Signal()
{
	return switch_thread_cond_signal(m_pCond);
}
//----------------------------------------------------------


/********************** end of TThreadCond ********************/



/************************ TThreadCondPool **************************/
TThreadCondPool :: TThreadCondPool(switch_memory_pool_t *pMemoryPool, unsigned int initCount) : m_pMemoryPool(pMemoryPool), m_mutex(pMemoryPool)
{
	for (int i = 0; i < initCount; ++i)
	{
		TThreadCond *pThreadCond = new TThreadCond(pMemoryPool);
		m_vecThreadConds.push_back(pThreadCond);
	}
}

TThreadCondPool :: ~TThreadCondPool()
{
	for (int i = 0; i < m_vecThreadConds.size(); ++i)
	{
		TThreadCond *pThreadCond = m_vecThreadConds.back();
		m_vecThreadConds.pop_back();
		delete pThreadCond;
	}
}

TThreadCond* TThreadCondPool :: GetThreadCond()
{
	TThreadCond *pThreadCond = NULL;

	m_mutex.Lock();
	if (m_vecThreadConds.size() > 0)
	{
		pThreadCond = m_vecThreadConds.back();
		m_vecThreadConds.pop_back();
		int size = m_vecThreadConds.size();
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "thread condition amount: %d\n", size);
	}
	m_mutex.UnLock();

	if (!pThreadCond)
	{
		return new TThreadCond(m_pMemoryPool);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "create new thread condition\n");
	}

	return pThreadCond;
}

void TThreadCondPool :: RecycleThreadCond(TThreadCond *threadCond)
{
	if (threadCond)
	{
		m_mutex.Lock();
		m_vecThreadConds.push_back(threadCond);
		int size = m_vecThreadConds.size();
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "thread condition amount: %d\n", size);
		m_mutex.UnLock();
	}
}

/******************** end of TThreadCondPool ***********************/