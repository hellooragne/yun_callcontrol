#ifndef MOD_CALLCONTROL_COMMANDEXUCUTOR_H_
#define MOD_CALLCONTROL_COMMANDEXUCUTOR_H_

#include "Controler.h"

const int NAME_LENGTH = 256;

#define recytime_of_find_channel 60

class TCmdExecutor
{
public:
	enum {
		UNKNOWN_CMD,
		MAKECALL_CMD,
		MAKECALL_UUI_CMD,
		HANGUP_CMD,
		ANSWER_CMD,
		SYNC_MAKECALL_CMD,
		SYNC_MAKECALL_UUI_CMD
	};

public:
	static void Execute(TManager &manager, const char *cmd, switch_stream_handle_t *stream);
	static const char* GetUsage();

private:
	static int ParseCmd(char *cmd, char *argv[], int iLen);
};

class TCommand 
{
public:
	TCommand(const char *sLocalId);
	virtual void Execute(TManager &manager, switch_stream_handle_t *stream);
	
protected:
	
	virtual bool Notify(TManager &manager, char *sId, int iLenOfId, char *sResult, int iLenOfRes);
	virtual bool Wait(TManager &manager, const char *sId, Handler *handler, char *sResult, int iLenOfRes);
	virtual void Response(TManager &manager, Handler *handler, char *sResult, int iLenOfRes);

protected:
	virtual void    GetContact(char *buf, int iLen);

	virtual void    GetWsContact(char *buf, int iLen);

	virtual void	GetEventBody(char *buf, int iLen) = 0;
	int				WaitForResponse(TManager &manager, const char *sId, Handler *handler);

	virtual void	ParseRetMsg(int iStatuCode, const char *sMsg, char *sResult, int iLen); 

	void			GetContactFromUrl(char *url, char *buf, int len);
protected:
	char m_sLocalId[NAME_LENGTH];
};


class TMakeCallCmd : public TCommand
{
public:
	TMakeCallCmd(const char *sCallerId, const char *sCalleeId);

private:
	void GetEventBody(char *buf, int iLen);
	
protected:
	char m_sRemoteId[NAME_LENGTH];
};


const int INFO_LENGTH = 1024;
class TMakeCallExCmd : public TCommand
{
public:
	TMakeCallExCmd(const char *sCallerId, const char *sCalleeId, const char *sExpandInfo);

private:
	void GetEventBody(char *buf, int iLen);

protected:
	char m_sRemoteId[NAME_LENGTH];
	char m_sExpandInfo[INFO_LENGTH];
};


class TSyncMakeCallCmd : public TMakeCallCmd
{
public:
	TSyncMakeCallCmd(const char *sCallerId, const char *sCalleeId);

protected:
	virtual void Response(TManager &manager, Handler *handler, char *sResult, int iLenOfRes);

protected:
	void GetUUID(char *buf, int len);

protected:
	switch_time_t m_beginTime;
};

class TSyncMakeCallExCmd : public TSyncMakeCallCmd
{
public:
	TSyncMakeCallExCmd(const char *sCallerId, const char *sCalleeId, const char *sExpandInfo);

private:
	void GetEventBody(char *buf, int iLen);

protected:
	char m_sExpandInfo[INFO_LENGTH];
};


class TAnswerCmd : public TCommand
{
public:
	TAnswerCmd(const char *sTargetId);

private:
	void GetEventBody(char *buf, int iLen);
};


class THangUpCmd : public TCommand
{
public:
	THangUpCmd(const char *sLocalId, const char *sRemoteId);

private:
	virtual void GetEventBody(char *buf, int iLen);

private:
	char m_sRemoteId[NAME_LENGTH];
};

#endif
