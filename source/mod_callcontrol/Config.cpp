#include "Config.h"
#include "switch.h"

TConfig::TConfig()
		: m_sProfileName(NULL)
		, m_sEventString(NULL)
		, m_sContentType(NULL)
		, m_sFromUser(NULL)
		, m_iTimeOut(3 * 1000 * 1000)
{
};

int TConfig::LoadConfig(const char* sFileName) 
{
	if (sFileName == NULL) 
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "mod_callcontrol use default config\n");
	}
	else
	{
		switch_xml_t cfg, xml, settings, param;
		if (!(xml = switch_xml_open_cfg(sFileName, &cfg, NULL))) 
		{
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Open of %s failed\n", sFileName);
		}
		else
		{
			if ((settings = switch_xml_child(cfg, "settings"))) {
				for (param = switch_xml_child(settings, "param"); param; param = param->next) {
					char *var = (char *) switch_xml_attr_soft(param, "name");
					char *val = (char *) switch_xml_attr_soft(param, "value");

					if (!strcasecmp(var, "profile")) 
					{
						int iLen = strlen(val);
						m_sProfileName = new char [iLen + 1];
						strncpy(m_sProfileName, val, iLen + 1);
					} 
					else if (!strcasecmp(var, "event-string")) 
					{
						int iLen = strlen(val);
						m_sEventString = new char [iLen + 1];
						strncpy(m_sEventString, val, iLen + 1);
					} 
					else if (!strcasecmp(var, "from-user"))
					{
						int iLen = strlen(val);
						m_sFromUser = new char [iLen + 1];
						strncpy(m_sFromUser, val, iLen + 1);
					}
				}
			}
		}

		switch_xml_free(xml);
	}

	//default settings
	int iLen;
	if (m_sProfileName == NULL)
	{
		iLen = strlen("internal");
		m_sProfileName = new char [iLen + 1];
		strncpy(m_sProfileName, "internal", iLen + 1);
	}

	if (m_sEventString == NULL)
	{
		iLen = strlen("ctrip-cti");
		m_sEventString = new char [iLen + 1];
		strncpy(m_sEventString, "ctrip-cti", iLen + 1);
	}

	if (m_sContentType == NULL)
	{
		iLen = strlen("application/xml");
		m_sContentType = new char [iLen + 1];
		strncpy(m_sContentType, "application/xml", iLen + 1);
	}

	if (m_sFromUser == NULL)
	{
		char *domain = switch_core_get_variable("domain");
		char *sFromUser = switch_mprintf("sip:ctrip_cti@%s", domain);
		iLen = strlen(sFromUser);
		m_sFromUser	= new char [iLen + 1];
		strncpy(m_sFromUser, sFromUser, iLen + 1);
		free(sFromUser);
	}

	return 0;

}
