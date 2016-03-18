#include "Handler.h"

Handler :: Handler() : m_iRetType(-1), m_pCond(NULL)
{
	m_sRetMsg[0] = 0;
}

void Handler :: Handle(int iType, const char *sMessage)
{
	m_iRetType = iType;
	if (sMessage)
	{
		strncpy(m_sRetMsg, sMessage, sizeof(m_sRetMsg));
	}

	if (m_pCond)
	{
		m_pCond->Signal();
	}
}

int Handler :: GetRetType()
{
	return m_iRetType;
}

char* Handler :: GetRetMessage()
{
	return m_sRetMsg;
}
void Handler :: SetThreadCond(TThreadCond *pCond)
{
	m_pCond = pCond;
}