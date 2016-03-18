#include "switch.h"
#include "CommandExecutor.h"

#include <string>
using namespace std;

const char* OK = "+OK\n";
const char* ERR_INVALID_ARGS = "-ERR invalid arguments\n";
const char* ERR_NO_USER = "-ERR user not registered\n";
const char* ERR_SERVER = "-ERR server error\n";
const char* ERR_RES_TIMEOUT = "-ERR response timeout out\n";
const char* ERR_CALL_NOT_EXIST = "-ERR not yet dial through or the call has beed hanguped by remote\n";
const char* ERR_UNKNOWN = "-ERR unknown error\n";

/************************* TCmdExecutor ******************************/
void TCmdExecutor :: Execute(TManager &manager, const char *cmd, switch_stream_handle_t *stream)
{
	//check cmd null or not null
	if (zstr(cmd))
	{
		if (stream)
		{
			stream->write_function(stream, "USAGE:%s", TCmdExecutor::GetUsage());
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "invalid arguments\n");
		}
		return;
	}

	char *arguments[4] = {0};
	char sCmdCopy[256];
	strncpy(sCmdCopy, cmd, sizeof(sCmdCopy));
	sCmdCopy[255] = 0;
	int iCmd;
	if ((iCmd = TCmdExecutor::ParseCmd(sCmdCopy, arguments, sizeof(arguments) / sizeof(arguments[0]))) == UNKNOWN_CMD)
	{
		if (stream)
		{
			stream->write_function(stream, ERR_INVALID_ARGS);
		}

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "invalid arguments\n");
		return;
	}

	TCommand *cmdInstance = NULL;
	switch (iCmd)
	{
	case MAKECALL_CMD:
		cmdInstance = new TMakeCallCmd(arguments[1], arguments[2]);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "API: makecall %s %s\n", arguments[1], arguments[2]);
		break;
	case MAKECALL_UUI_CMD:
		cmdInstance = new TMakeCallExCmd(arguments[1], arguments[2], arguments[3]);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "API: makecall %s %s %s\n", arguments[1], arguments[2], arguments[3]);
		break;
	case ANSWER_CMD:
		cmdInstance = new TAnswerCmd(arguments[1]);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "API: answer %s\n", arguments[1]);
		break;
	case HANGUP_CMD:
		cmdInstance = new THangUpCmd(arguments[1], arguments[2]);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "API: hangup %s\n", arguments[1]);
		break;
	case SYNC_MAKECALL_CMD:
		cmdInstance = new TSyncMakeCallCmd(arguments[1], arguments[2]);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "API: syncmakecall %s %s\n", arguments[1], arguments[2]);
		break;
	case SYNC_MAKECALL_UUI_CMD:
		cmdInstance = new TSyncMakeCallExCmd(arguments[1], arguments[2], arguments[3]);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "API: syncmakecall %s %s %s\n", arguments[1], arguments[2], arguments[3]);
		break;
	default:
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "invalid arguments\n");
		stream->write_function(stream, ERR_INVALID_ARGS);
		return;
	}

	cmdInstance->Execute(manager, stream);

	delete cmdInstance;
}

const char* TCmdExecutor :: GetUsage()
{
	return
		"callcontrol MakeCall [from number] [to number] | \n" 
		"callcontrol MakeCall [from number] [to number] [X-UUI] | \n" 
		"callcontrol HangUp [local number] [remote number] | \n" 
		"callcontrol Answer [local number] | \n"
		"callcontrol SyncMakeCall [from number] [to number] | \n" 
		"callcontrol SyncMakeCall [from number] [to number] [X-UUI] | \n" ;
}

int TCmdExecutor :: ParseCmd(char *cmd, char *argv[], int iLen)
{
	int argc = switch_separate_string(cmd, ' ', argv, iLen);

	const char *sCmdType = argv[0];

	if (!strcasecmp(sCmdType, "MakeCall"))
	{
		if (argc == 3)
		{
			return MAKECALL_CMD;
		}
		if (argc == 4)
		{
			return MAKECALL_UUI_CMD;
		}
	} 
	else if (!strcasecmp(sCmdType, "HangUp"))
	{
		if (argc >= 3)
		{
			return HANGUP_CMD;
		}
	}
	else if (!strcasecmp(sCmdType, "Answer"))
	{
		if (argc >= 2)
		{
			return ANSWER_CMD;
		}
	}
	else if (!strcasecmp(sCmdType, "SyncMakeCall"))
	{
		if (argc == 3)
		{
			return SYNC_MAKECALL_CMD;
		}
		if (argc == 4)
		{
			return SYNC_MAKECALL_UUI_CMD;
		}
	}

	return UNKNOWN_CMD;
}

/********************** end of TCmdExecutor ***************************/


/****************************** TCommand *************************************/
TCommand :: TCommand(const char *sLocalId) 
{
	if (sLocalId) 
	{ 
		int iLen = sizeof(m_sLocalId);
		strncpy(m_sLocalId, sLocalId, iLen);
		m_sLocalId[iLen - 1] = 0;
	}
	else 
	{ 
		m_sLocalId[0] = 0;
	}
}

void TCommand :: Execute(TManager &manager, switch_stream_handle_t *stream)
{
	char sId[SWITCH_UUID_FORMATTED_LENGTH + 1] = { 0 };
	char sResult[1024] = { 0 };
	if (Notify(manager, sId, sizeof(sId), sResult, sizeof(sResult)))
	{
		Handler handler;
		if (Wait(manager, sId, &handler, sResult, sizeof(sResult)))
		{
		    
			Response(manager, &handler, sResult, sizeof(sResult));
		}
	}

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "RETURN: %s\n", sResult);
	stream->write_function(stream, sResult);
}
//
//bool TCommand :: Notify(TManager &manager, char *sId, int iLen, switch_stream_handle_t *stream)
bool TCommand :: Notify(TManager &manager, char *sId, int iLenOfId, char *sResult, int iLenOfRes)
{
	//init config data
	char sContact[1024] = { 0 };
	char cContact[1024] = { 0 };
	GetContact(sContact, sizeof(sContact));
	GetWsContact(cContact, sizeof(cContact));

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "NORMAL: Notify device contact: %s\n", sContact);

	if (strlen(sContact) == 0)
	{
		strncpy(sResult, ERR_NO_USER, iLenOfRes);
		sResult[iLenOfRes - 1] = 0;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "EXCEPTION: Fail to notify device for reason that can't get device's contact\n");	
		return false;
	}

	switch_uuid_str(sId, iLenOfId);
	char *sProfileName = manager.GetProfileName();
	char *sEventString = manager.GetEventString();
	char *sContentType = manager.GetContentType();
	char *sFromUser = manager.GetFromUser();
	char sExHeader[512] = "token:";
	strncat(sExHeader, sId, sizeof(sExHeader) - strlen(sExHeader));
	char sBody[1024] = { 0 };
	GetEventBody(sBody, sizeof(sBody));
	
	//send notify event
	int	iBodyLength	 = strlen(sBody);

	switch_event_t *custom_event;
	switch_event_create(&custom_event, SWITCH_EVENT_NOTIFY);
	switch_event_add_header_string(custom_event, SWITCH_STACK_BOTTOM, "from-uri",		sFromUser);
	switch_event_add_header_string(custom_event, SWITCH_STACK_BOTTOM, "to-uri",			sContact);
	switch_event_add_header_string(custom_event, SWITCH_STACK_BOTTOM, "profile",			sProfileName);
	switch_event_add_header_string(custom_event, SWITCH_STACK_BOTTOM, "event-string",	sEventString);
	switch_event_add_header_string(custom_event, SWITCH_STACK_BOTTOM, "content-type",	sContentType);
	switch_event_add_header_string(custom_event, SWITCH_STACK_BOTTOM, "extra-headers",	sExHeader);
	switch_event_add_header_string(custom_event, SWITCH_STACK_BOTTOM, "contact-uri",	cContact);
	switch_event_add_header(custom_event, SWITCH_STACK_BOTTOM, "content-length", "%d",	iBodyLength);
	
	switch_event_add_body(custom_event, "%s", sBody);

	switch_event_fire(&custom_event);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "fire notify event\n");
	return true;
}

//bool TCommand :: Wait(TManager &manager, char *sId, Handler *handler, switch_stream_handle_t *stream)
bool TCommand :: Wait(TManager &manager, const char *sId, Handler *handler, char *sResult, int iLenOfRes)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "NORMAL: 2.Waiting for device's response\n");
	//wait for response
	int iRet;
	iRet = WaitForResponse(manager, sId, handler);
	if (iRet != SWITCH_STATUS_SUCCESS)
	{
		if (iRet == SWITCH_STATUS_TIMEOUT)
		{
			strncpy(sResult, ERR_RES_TIMEOUT, iLenOfRes);
			sResult[iLenOfRes - 1] = 0;
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Wait for response timeout\n");
		}
		else
		{
			strncpy(sResult, ERR_UNKNOWN, iLenOfRes);
			sResult[iLenOfRes - 1] = 0;
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Wait for response fail with unknown reason\n");
		}
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "EXCEPTION: Fail to get device's response\n");
		return false;
	}
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "EXCEPTION: success to get device's response,iRet value: \n",iRet);
	return true;
}

//void TCommand :: Response(TManager &manager, Handler *handler, switch_stream_handle_t *stream)
void TCommand :: Response(TManager &manager, Handler *handler, char *sResult, int iLenOfRes)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "NORMAL: 3.Got device's response\n");
	ParseRetMsg(handler->GetRetType(), handler->GetRetMessage(), sResult, iLenOfRes);
}

int TCommand :: WaitForResponse(TManager &manager, const char *sId, Handler *handler)
{
	TThreadCond *threadCond = manager.GetThreadCond();
	if (!threadCond) 
	{
		return SWITCH_STATUS_FALSE;
	}

	handler->SetThreadCond(threadCond);
	manager.Subscribe(sId, handler);

	int iRet;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "start waiting...\n");
	iRet = threadCond->Wait(manager.GetTimeOut());
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "end waiting\n");

	manager.Unsubscribe(sId);
	handler->SetThreadCond(NULL);
	manager.ReleaseThreadCond(threadCond);

	return iRet;
}

void TCommand :: GetContact(char *buf, int iLen)
{
	//get contact string
	switch_core_flag_t cflags = switch_core_flags();
	if (!(cflags & SCF_USE_SQL)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "-ERR SQL disabled, no data available!\n");
		return;
	}
	
	switch_cache_db_handle_t *db = NULL;
	if (switch_core_db_handle(&db) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "-ERR Database error!\n");
		return;
	}

	char *sql = switch_mprintf("select url from registrations where reg_user='%s'", m_sLocalId);
	char ret[1024];

	switch_cache_db_execute_sql2str(db, sql, ret, sizeof(ret), NULL);

	switch_safe_free(sql);
	if (db) { switch_cache_db_release_db_handle(&db); }

	//extract contact string
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "device url: %s\n", ret);

	GetContactFromUrl(ret, buf, iLen);
}


void TCommand :: GetWsContact(char *buf, int iLen) {

	switch_core_flag_t cflags = switch_core_flags();
	if (!(cflags & SCF_USE_SQL)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "-ERR SQL disabled, no data available!\n");
		return;
	}
	
	switch_cache_db_handle_t *db = NULL;

	if (switch_cache_db_get_db_handle_dsn(&db, "sofia_reg_internal") != SWITCH_STATUS_SUCCESS) {

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "db handle dsn error\n");
		db = NULL;
		return;
	}

	char *sql = switch_mprintf("select contact from sip_registrations where sip_user='%s' limit 1", m_sLocalId);

	char ret[1024];

	switch_cache_db_execute_sql2str(db, sql, ret, sizeof(ret), NULL);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "sql %s\n", sql);

	switch_safe_free(sql);
	if (db) { switch_cache_db_release_db_handle(&db); }

	//extract contact string
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "device contact: %s\n", ret);

	strcpy(buf, ret);
}

void TCommand :: GetContactFromUrl(char *url, char *buf, int len)
{
	if (zstr(url))
	{
		buf[0] = 0;
		return;
	}

	string strUrl(url);
	string::size_type lPos;
	lPos = strUrl.find("fs_path");
	//lPos = strUrl.find("fs_nat");
	if (lPos != string::npos)
	{

		int offset = 0;
		lPos += 8;

		while (lPos < strUrl.size() && strUrl[lPos] != '%')
		{
			buf[offset++] = strUrl[lPos++];
		}

		buf[offset++] = ':';
		lPos += 3;

		while (lPos < strUrl.size() && strUrl[lPos] != '%')
		{
			buf[offset++] = strUrl[lPos++];
		}

		buf[offset++] = '@';
		lPos += 3;

		while (lPos < strUrl.size() && strUrl[lPos] != '%')
		{
			buf[offset++] = strUrl[lPos++];
		}

		buf[offset++] = ':';
		lPos += 3;

		while (lPos < strUrl.size() && strUrl[lPos] != ';')
		{
			buf[offset++] = strUrl[lPos++];
		}

		buf[offset] = '\0';


		string::size_type lPos_n;

		lPos_n = string(buf).find("%");

		buf[lPos_n] = '\0';

		//strcpy(buf, "sip:6je272uo@0231krab06hk.invalid");
	}
	else
	{
		lPos = strUrl.find("sip:");
		int offset = 0;
		while (lPos < strUrl.size() && strUrl[lPos] != ';')
		{
			buf[offset++] = strUrl[lPos++];
		}

		buf[offset] = '\0';
	}
}

void TCommand :: ParseRetMsg(int iStatuCode, const char *sMsg, char *sResult, int iLen)
{
	if (iStatuCode == 200)
	{
		strncpy(sResult, OK, iLen);
		sResult[iLen - 1] = 0;
	}
	else
	{
		if (!zstr(sMsg))
		{
			char sMsgDup[1024] = { 0 };
			strncpy(sMsgDup, sMsg, sizeof(sMsgDup));
			sMsgDup[sizeof(sMsgDup) - 1] = 0;

			switch_xml_t xml;
			xml = switch_xml_parse_str(sMsgDup, strlen(sMsgDup));
			if (xml->txt != NULL)
			{
				strncpy(sResult, xml->txt, iLen);
				sResult[iLen - 1] = 0;
			}
			else
			{
				strncpy(sResult, ERR_UNKNOWN, iLen);
				sResult[iLen - 1] = 0;
			}
			switch_xml_free(xml);
		}
		else
		{
			strncpy(sResult, ERR_UNKNOWN, iLen);
			sResult[iLen - 1] = 0;
		}
	}	
}
/************************* end of TCommand ***************************/


/************************* TMakeCallCmd *****************************/
TMakeCallCmd :: TMakeCallCmd(const char *sCallerId, const char *sCalleeId)
	: TCommand(sCallerId)
{
	if (sCalleeId) 
	{
		int iLen = sizeof(m_sRemoteId);
		strncpy(m_sRemoteId, sCalleeId, iLen);
		m_sRemoteId[iLen -1] = 0;
	}
	else
	{
		m_sRemoteId[0] = 0;
		
	}
}

void TMakeCallCmd :: GetEventBody(char *buf, int iLen)
{
	char *sBody = switch_mprintf(
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<cmd Dnis=\"%s\">" 
		"MakeCall"
		"</cmd>\n"
		, m_sRemoteId);

	strncpy(buf, sBody, iLen);
	buf[iLen - 1] = 0;

	switch_safe_free(sBody);
}

/********************* end of TMakeCmd ******************************/


/************************* TMakeCallCmd *****************************/
TAnswerCmd :: TAnswerCmd(const char *sTargetId) : TCommand(sTargetId)
{
}

void TAnswerCmd :: GetEventBody(char *buf, int iLen)
{
	const char *sBody = 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<cmd>" 
		"Answer"
		"</cmd>\n";

	strncpy(buf, sBody, iLen);
	buf[iLen - 1] = 0;
}

/********************* end of TAnswerCmd ******************************/


/************************* THangUpCmd *****************************/
THangUpCmd :: THangUpCmd(const char *sLocalId, const char *sRemoteId)
	: TCommand(sLocalId)
{
	if (sRemoteId) 
	{
		int iLen = sizeof(m_sRemoteId);
		strncpy(m_sRemoteId, sRemoteId, iLen);
		m_sRemoteId[iLen - 1] = 0;
	}
	else
	{
		m_sRemoteId[0] = 0;
	}
}

void THangUpCmd :: GetEventBody(char *buf, int iLen)
{
	char *sBody = switch_mprintf(
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<cmd RemoteId=\"%s\" >" 
		"HangUp"
		"</cmd>\n",
		m_sRemoteId);

	strncpy(buf, sBody, iLen);
	buf[iLen - 1] = 0;

	switch_safe_free(sBody);
}

/********************* end of THangUpCmd ******************************/


/************************* TMakeCallExCmd *****************************/
TMakeCallExCmd :: TMakeCallExCmd(const char *sCallerId, const char *sCalleeId, const char *sExpandInfo) : TCommand(sCallerId)
{
	if (!sCalleeId || !sExpandInfo) 
	{
		m_sRemoteId[0] = 0;
		m_sExpandInfo[0] = 0;
	}
	else
	{	
		int iLenOfRemoteId = sizeof(m_sRemoteId);
		int iLenOfInfo = sizeof(m_sExpandInfo);
		strncpy(m_sRemoteId, sCalleeId, iLenOfRemoteId);
		strncpy(m_sExpandInfo, sExpandInfo, iLenOfInfo);
		m_sRemoteId[iLenOfRemoteId - 1] = 0;
		m_sExpandInfo[iLenOfInfo - 1] = 0;
	}
}

void TMakeCallExCmd :: GetEventBody(char *buf, int iLen)
{
	char *sBody = switch_mprintf(
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<cmd Dnis=\"%s\" X-UUI=\"%s\">" 
		"MakeCall"
		"</cmd>\n",
		m_sRemoteId, m_sExpandInfo);

	strncpy(buf, sBody, iLen);
	buf[iLen - 1] = 0;

	switch_safe_free(sBody);
}

/********************* end of THangUpCmd ******************************/


/*************************** TSyncMakeCallCmd ****************************/
TSyncMakeCallCmd :: TSyncMakeCallCmd(const char *sCallerId, const char *sCalleeId) : TMakeCallCmd(sCallerId, sCalleeId)
{
	m_beginTime = switch_time_now() / 1000000;
}

void TSyncMakeCallCmd :: Response(TManager &manager, Handler *handler, char *sResult, int iLenOfRes)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "TSyncMakeCallCmd response NORMAL: 3.Got device's response\n");

	int retStatus = handler->GetRetType();
	if (retStatus == 200) //ok then wait
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "NORMAL: 4.Check the call state\n");
		//get uuid
		int count = 0;
		char uuid[256] = { 0 };
		
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "get uuid channels: %s\n",uuid);

		GetUUID(uuid, sizeof(uuid));
		while(count++ < recytime_of_find_channel && uuid[0] == 0) 
		{
			switch_yield(100000); 
			GetUUID(uuid, sizeof(uuid));	
			//add for test by yuecheng
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "TSyncMakeCallCmd :: Response,get uuid channels: %s, count: %d\n",uuid,count);
		}

		if (strlen(uuid) == 0)
		{
			strncpy(sResult, ERR_CALL_NOT_EXIST, iLenOfRes);
			sResult[iLenOfRes - 1] = 0;
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "EXCEPTION: Not yet dial through dnis, target call is not exist\n");
			return;
		} 
		else
		{
			char res[256] = "+OK ";
			strcat(res, uuid);
			strncpy(sResult, res, iLenOfRes);
			sResult[iLenOfRes - 1] = 0;
			switch_yield(500000);  //protect from return too fast
			return;
		}
	}
	else // not ok directly return 
	{
		ParseRetMsg(retStatus, handler->GetRetMessage(), sResult, iLenOfRes);
	}
}

void TSyncMakeCallCmd :: GetUUID(char *buf, int len)
{
		//get contact string
	switch_core_flag_t cflags = switch_core_flags();
	if (!(cflags & SCF_USE_SQL)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "-ERR SQL disabled, no data available!\n");
		return;
	}
	
	switch_cache_db_handle_t *db = NULL;
	if (switch_core_db_handle(&db) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "-ERR Database error!\n");
		return;
	}

	//add for test by yuecheng
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "get uuid by cid num: %s, dest : %s\n",m_sLocalId,m_sRemoteId);


	//char *sql = switch_mprintf("select uuid from channels where cid_num='%s' and dest='%s' and direction='inbound' and created_epoch >= '%"SWITCH_TIME_T_FMT"' and (callstate='EARLY' or callstate='ACTIVE')", m_sLocalId, m_sRemoteId, m_beginTime);
		
	char *sql = switch_mprintf("select uuid from channels where cid_num='%s' and dest='%s' and direction='inbound' and created_epoch >= '%"SWITCH_TIME_T_FMT"' and (callstate='EARLY' or callstate='ACTIVE')", m_sLocalId, m_sRemoteId, m_beginTime);

	switch_cache_db_execute_sql2str(db, sql, buf, len, NULL);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "GetUUID buf : %s,  m_beginTime time: %"SWITCH_TIME_T_FMT"\n",buf,m_beginTime);

	switch_safe_free(sql);
	if (db) { switch_cache_db_release_db_handle(&db); }
}

/************************* end of TCommand ***************************/


/*************************** TSyncMakeCallExCmd ****************************/
TSyncMakeCallExCmd :: TSyncMakeCallExCmd(const char *sCallerId, const char *sCalleeId, const char *sExpandInfo) : TSyncMakeCallCmd(sCallerId, sCalleeId)
{
	if (!sExpandInfo) 
	{
		m_sExpandInfo[0] = 0;
	}
	else
	{	
		int iLen = sizeof(m_sExpandInfo);
		strncpy(m_sExpandInfo, sExpandInfo, iLen);
		m_sExpandInfo[iLen - 1] = 0;
	}
}

void TSyncMakeCallExCmd :: GetEventBody(char *buf, int iLen)
{
	char *sBody = switch_mprintf(
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<cmd Dnis=\"%s\" X-UUI=\"%s\">" 
		"MakeCall"
		"</cmd>\n",
		m_sRemoteId, m_sExpandInfo);

	strncpy(buf, sBody, iLen);
	buf[iLen - 1] = 0;

	switch_safe_free(sBody);
}
/************************ end of TSyncMakeCallExCmd ************************/
