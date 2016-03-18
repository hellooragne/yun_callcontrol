#ifndef MOD_CALLCONTROL_THREADSYNC_H_
#define MOD_CALLCONTROL_THREADSYNC_H_

#include "switch.h"
#include <vector>
//#include "Common.h"

/*****************************************
 * description: a better way to user mutex
 */
class TMutex
{
public: 
	TMutex(switch_memory_pool_t *pMemoryPool);
	~TMutex();
	switch_status_t Lock();
	switch_status_t UnLock();

private:
	switch_mutex_t	*m_pSwitchMutex;
};


/********************************************
 * description: a better way to user condition
 */
class TThreadCond
{
public:
	TThreadCond(switch_memory_pool_t *pMemoryPool);
	~TThreadCond();
	switch_status_t Wait(int iTimeout);
	switch_status_t Signal();

private:
	switch_mutex_t			*m_pMutex;
	switch_thread_cond_t	*m_pCond;
};


/************************************************
 * description: cond pool
 */
class TThreadCondPool
{
public:
	TThreadCondPool(switch_memory_pool_t *pMemoryPool, unsigned int initCount);
	~TThreadCondPool();

	TThreadCond* GetThreadCond();
	void RecycleThreadCond(TThreadCond *threadCond);

private:
	switch_memory_pool_t *m_pMemoryPool;
	TMutex m_mutex;
	std::vector <TThreadCond *> m_vecThreadConds;
};

#endif