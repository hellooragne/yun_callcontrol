
#include "ThreadUtil.h"

enum { MSG_LEN =1024 };

class Handler
{
public:
	Handler();
	void Handle(int iType, const char *sMessage);
	int GetRetType();
	char *GetRetMessage();
	void SetThreadCond(TThreadCond *pCond);

private:
	int m_iRetType;
	char m_sRetMsg[MSG_LEN];
	TThreadCond *m_pCond;
};