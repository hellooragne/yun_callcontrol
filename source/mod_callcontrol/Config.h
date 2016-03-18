#ifndef MOD_CALLCONTROL_DATASTRUCT_H_
#define MOD_CALLCONTROL_DATASTRUCT_H_

class TThreadSyncTool;
typedef struct DataPackage
{
	TThreadSyncTool *syncTool;
	char			*sDescription;
	int				iStatusCode;
} DataPackage_t;


/****************************
 * description: configuration
 */
class TConfig
{
public:
	TConfig();

	~TConfig()
	{
		delete [] m_sProfileName;
		delete [] m_sEventString;
		delete [] m_sContentType;
		delete [] m_sFromUser;
	}

	int LoadConfig(const char* sFileName);

	char* GetProfileName() { return m_sProfileName; };
	char* GetEventString() { return m_sEventString; };
	char* GetContentType() { return m_sContentType; };
	char* GetFromUser() { return m_sFromUser; };
	int   GetTimeOut() { return m_iTimeOut; };

private:
	char *m_sProfileName;
	char *m_sEventString;
	char *m_sContentType;
	char *m_sFromUser;
	int	 m_iTimeOut;

};

#endif