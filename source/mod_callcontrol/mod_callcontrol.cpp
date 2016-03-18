/* 
 *	Name:			mod_callcontrol
 *	Description:	Through sending notify sip signaling to control devices
 *	Author:			djxie
 *	Version:		1.2.23_20150601.01
 *	Date:			2015.5.29
 */


#include "switch.h"
#include "Controler.h"
#include "CommandExecutor.h"

using namespace std;

extern "C" {
/* Prototypes */
SWITCH_MODULE_LOAD_FUNCTION(mod_callcontrol_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_callcontrol_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_callcontrol_runtime);

/* SWITCH_MODULE_DEFINITION(name, load, shutdown, runtime) 
 * Defines a switch_loadable_module_function_table_t and a static const char[] modname
 */
SWITCH_MODULE_DEFINITION(mod_callcontrol, mod_callcontrol_load, mod_callcontrol_shutdown, NULL);
}

//static TControler g_controler;
static TManager *g_pManager = NULL;

void OnResponseMessageIn(switch_event_t *pEvent)
{
	const char *sToken;
	const char *sBody;
	const char *sStatus;

	sToken = switch_event_get_header(pEvent, "token");
	sStatus = switch_event_get_header(pEvent, "status");
	sBody = switch_event_get_body(pEvent);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "coming ctrip_event:\ntoken: %s\nstatus: %s\nbody: %s\n\n", sToken, sStatus, sBody);

	if (sToken != NULL && sStatus != NULL)
	{
		int iStatus = atoi(sStatus);
		//g_controler.GetMessageCenter().DispatchMessage(sToken, iStatus, sBody);
		if (g_pManager)
		{
			g_pManager->HandleEvent(sToken, iStatus, sBody);
		}
	}
}

SWITCH_STANDARD_API(callcontrol_api_function)
{
	if (g_pManager)
	{
		g_pManager->ExecuteCmd(cmd, stream);
	}
	else
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "fail to call mod_callcontrol api, because of manager is error\n");
	}

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_callcontrol_load)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "loading mod_callcontrol...\n");
	g_pManager = new TManager(pool);
	g_pManager->LoadConfig("callcontrol.conf");
	g_pManager->Work();

	switch_api_interface_t *api_interface;
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(api_interface, "callcontrol", "callcontrol API", callcontrol_api_function, TCmdExecutor::GetUsage());

	if (switch_event_bind(modname, SWITCH_EVENT_CUSTOM, "ctrip_event", OnResponseMessageIn, NULL) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "fail to bind switch event:ctrip_event\n");
		return SWITCH_STATUS_GENERR;
	}

	switch_console_set_complete("add callcontrol MakeCall [from number] [to number]");
	switch_console_set_complete("add callcontrol MakeCall [from number] [to number] [X-UUI]");
	switch_console_set_complete("add callcontrol SyncMakeCall [from number] [to number]");
	switch_console_set_complete("add callcontrol SyncMakeCall [from number] [to number] [X-UUI]");
	switch_console_set_complete("add callcontrol HangUp [local number] [remote number]");
	switch_console_set_complete("add callcontrol Answer [local number]");

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "mod_callcontrol loaded\n");
	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
	
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_callcontrol_shutdown)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "shut down mod_callcontrol ing...\n");

	switch_event_unbind_callback(OnResponseMessageIn);

	if (g_pManager)
	{
		g_pManager->Shutdown();
		delete g_pManager;
		g_pManager = NULL;
	}	

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "shut down mod_callcontrol OK\n");

	return SWITCH_STATUS_SUCCESS;
}
