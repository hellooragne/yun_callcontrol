#include "Controler.h"
#include "CommandExecutor.h"


/*********************** NewTControlser *********************/
TManager :: TManager(switch_memory_pool_t *pMemoryPool) 
	: m_isRunning(false)
	, m_iThreadAmount(0)
	, m_pMemoryPool(pMemoryPool)
	, m_mutex(pMemoryPool)
	, m_condPool(pMemoryPool, 256)
	, m_notification(pMemoryPool)
{
}

TManager :: ~TManager()
{
}

void TManager :: ExecuteCmd(const char *cmd, switch_stream_handle_t *stream)
{
	m_mutex.Lock();
	if (!m_isRunning)
	{
		m_mutex.UnLock();
		stream->write_function(stream, "-ERR system is down\n");
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "mod_callcontrol has been stop\n");
		return;
	}
	else
	{
		m_iThreadAmount++;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "mod_callcontrol work thread amount: %d\n", m_iThreadAmount);
	}
	m_mutex.UnLock();

	TCmdExecutor::Execute(*this, cmd, stream);

	m_mutex.Lock();
	m_iThreadAmount--;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "mod_callcontrol work thread amount: %d\n", m_iThreadAmount);
	m_mutex.UnLock();
}

void TManager :: HandleEvent(const char *sId, int iType, const char *sMsg)
{
	m_notification.Dispatch(sId, iType, sMsg);
}

int TManager :: Subscribe(const char *sId, Handler *handler)
{
	return m_notification.Subscribe(sId, handler);
}

int TManager :: Unsubscribe(const char *sId)
{
	return m_notification.Unsubscribe(sId);
}

void TManager :: LoadConfig(const char *sFileName)
{
	m_config.LoadConfig(sFileName);
}

char* TManager :: GetProfileName()
{
	return m_config.GetProfileName();
}

char* TManager :: GetEventString()
{
	return m_config.GetEventString();
}

char* TManager :: GetContentType()
{
	return m_config.GetContentType();
}

char* TManager :: GetFromUser()
{
	return m_config.GetFromUser();
}


TThreadCond* TManager :: GetThreadCond()
{
	return m_condPool.GetThreadCond();
}

int TManager :: GetTimeOut()
{
	return m_config.GetTimeOut();
}

void TManager :: ReleaseThreadCond(TThreadCond *threadCond)
{
	m_condPool.RecycleThreadCond(threadCond);
}

void TManager :: Work()
{
	m_mutex.Lock();
	m_isRunning = true;
	m_mutex.UnLock();
}

void TManager :: Stop()
{
	m_mutex.Lock();
	m_isRunning = false;
	m_mutex.UnLock();
}

void TManager :: Shutdown()
{
	Stop();

	int count = 0;

	//wait the running thread to finish their work, 
	//force to stop when waited for 10 times
	
	while (count++ < 40)
	{
		bool canShutdown = false;

		m_mutex.Lock();
		if (m_iThreadAmount <= 0)
		{
			canShutdown = true;
		}
		m_mutex.UnLock();

		if (canShutdown)
		{
			break;
		}

		switch_yield(500000);
	}	
}

/***************** end of TManager *************************/