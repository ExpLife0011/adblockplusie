#include "AdPluginStdAfx.h"

#include "AdPluginConfig.h"
#include "AdPluginClient.h"
#include "AdPluginSettings.h"
#include "AdPluginMutex.h"


class CPluginConfigLock : public CPluginMutex
{
public:
    CPluginConfigLock() : CPluginMutex("ConfigFile", PLUGIN_ERROR_MUTEX_CONFIG_FILE) {}
    ~CPluginConfigLock() {}
};


CPluginConfig* CPluginConfig::s_instance = NULL;

CComAutoCriticalSection CPluginConfig::s_criticalSection;


CPluginConfig::CPluginConfig()
{
	DEBUG_GENERAL("*** Initializing config")

    bool isExisting = true;
    {
        CPluginConfigLock lock;
        if (lock.IsLocked())
        {
	        std::ifstream is;
	        is.open(CPluginSettings::GetDataPath(CONFIG_INI_FILE), std::ios_base::in);
	        if (!is.is_open())
	        {
	            isExisting = false;
	        }
	        else
	        {
		        is.close();
	        }
        }
    }

    if (!isExisting)
    {
	    Create();
    }
	Read();
}


CPluginConfig::~CPluginConfig()
{
    s_criticalSection.Lock();
	{
		s_instance = NULL;
	}
    s_criticalSection.Unlock();
}


CPluginConfig* CPluginConfig::GetInstance() 
{
	CPluginConfig* instance = NULL;

    s_criticalSection.Lock();
	{
		if (!s_instance)
		{
			s_instance = new CPluginConfig();
		}

		instance = s_instance;
	}
    s_criticalSection.Unlock();

	return instance;
}


void CPluginConfig::Read()
{
	DEBUG_GENERAL("*** Loading config:" + CPluginSettings::GetDataPath(CONFIG_INI_FILE))

    CPluginConfigLock lock;
    if (lock.IsLocked())
    {
        CPluginIniFile iniFile(CPluginSettings::GetDataPath(CONFIG_INI_FILE), false);

        if (!iniFile.Read())
        {
            DEBUG_ERROR_LOG(iniFile.GetLastError(), PLUGIN_ERROR_CONFIG, PLUGIN_ERROR_CONFIG_READ_FILE, "Dictionary::SetLanguage - Read")
            return;
        }

        s_criticalSection.Lock();
	    {
            m_downloadFileProperties.clear();
			m_downloadFileCategories.clear();

			const CPluginIniFile::TSectionNames& names = iniFile.GetSectionNames();

			for (CPluginIniFile::TSectionNames::const_iterator it = names.begin(); it != names.end(); ++it)
			{
				if (it->Left(8) == L"category")
				{
					CPluginIniFile::TSectionData data = iniFile.GetSectionData(*it);

					SDownloadFileCategory category;

					category.description = data["description"];
					category.extension = data["extension"];
					category.ffmpegArgs = data["ffmpeg"];
					category.type = data["type"];
					category.category = *it;

					m_downloadFileCategories[category.category] = category;
				}
				else if (it->Left(6) == L"format")
				{
					CPluginIniFile::TSectionData data = iniFile.GetSectionData(*it);

					CString contents = data["type"];

					int pos = 0;

					CString content = contents.Tokenize(L";", pos);
					while (pos >= 0)
					{
						SDownloadFileProperties properties;

						properties.category = data["category"];
						properties.conversions = data["conversions"];
						properties.content = content;

						TDownloadFileCategories::const_iterator it = m_downloadFileCategories.find(properties.category);
						if (it != m_downloadFileCategories.end())
						{
							properties.properties = it->second;
						}

						m_downloadFileProperties[content] = properties;

						content = contents.Tokenize(L";", pos);
					}
				}
			}
	    }
        s_criticalSection.Unlock();
    }
}


void CPluginConfig::Create()
{
	DEBUG_GENERAL("*** Creating config:" + CPluginSettings::GetDataPath(CONFIG_INI_FILE));

    CPluginConfigLock lock;
    if (lock.IsLocked())
    {
        CPluginIniFile iniFile(CPluginSettings::GetDataPath(CONFIG_INI_FILE), false);

        CPluginSettings* settings = CPluginSettings::GetInstance();

        if (iniFile.Exists() || !settings->IsMainProcess())
        {
            return;
        }

        s_criticalSection.Lock();
	    {
			// Formats
			// ----------------------------------------------------------------
			// .asf
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "video/x-ms-asf";
				format["category"] = "categoryAsf";
				format["conversions"] = "Video;Audio";

				iniFile.UpdateSection("formatAsf", format);
			}
			// .avi
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "video/avi;video/msvideo;video/x-msvideo";
				format["category"] = "categoryAvi";
				format["conversions"] = "Video;Audio";

				iniFile.UpdateSection("formatAvi", format);
			}
			// .flv
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "video/x-flv";
				format["category"] = "categoryFlv";
				format["conversions"] = "Video;Audio";

				iniFile.UpdateSection("formatFlv", format);
			}
			// .mov
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "video/quicktime";
				format["category"] = "categoryMov";
				format["conversions"] = "Video;Audio";

				iniFile.UpdateSection("formatMov", format);
			}
			// .mp3
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "audio/mpeg";
				format["category"] = "categoryMp3";
				format["conversions"] = "Audio";

				iniFile.UpdateSection("formatMp3", format);
			}
			// .mp4 audio
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "audio/mp4";
				format["category"] = "categoryMp4Audio";
				format["conversions"] = "Audio";

				iniFile.UpdateSection("formatMp4Audio", format);
			}
			// .mp4 video
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "video/mp4";
				format["category"] = "categoryMp4Video";
				format["conversions"] = "Video;Audio";

				iniFile.UpdateSection("formatMp4Video", format);
			}
			// .wav
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "audio/x-wav;audio/wav;audio/wave";
				format["category"] = "categoryWav";
				format["conversions"] = "Audio";

				iniFile.UpdateSection("formatWav", format);
			}
			// .wmv
			{
    			CPluginIniFile::TSectionData format;

				format["type"] = "video/x-ms-wmv";
				format["category"] = "categoryWmv";
				format["conversions"] = "";

				iniFile.UpdateSection("formatWmv", format);
			}

			// Categories
			// ----------------------------------------------------------------
			// asf
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Video";
				category["extension"] = "asf";
				category["description"] = "Advanced Systems Format";
				category["ffmpeg"] = "";

				iniFile.UpdateSection("categoryAsf", category);
			}
			// avi
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Video";
				category["extension"] = "avi";
				category["description"] = "Audio Video Interleave";
				category["ffmpeg"] = "";

				iniFile.UpdateSection("categoryAvi", category);
			}
			// flv
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Video";
				category["extension"] = "flv";
				category["description"] = "Flash Video";
				category["ffmpeg"] = "";

				iniFile.UpdateSection("categoryFlv", category);
			}
			// mp3
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Audio";
				category["extension"] = "mp3";
				category["description"] = "MP3";
				category["ffmpeg"] = "-acodec libmp3lame -ab 160kb -ac 2 -ar 44100";

				iniFile.UpdateSection("categoryMp3", category);
			}
			// mov
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Video";
				category["extension"] = "mov";
				category["description"] = "QuickTime";
				category["ffmpeg"] = "";

				iniFile.UpdateSection("categoryMov", category);
			}
			// mp4 Audio
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Audio";
				category["extension"] = "mp4";
				category["description"] = "MPEG-4 Audio";
				category["ffmpeg"] = "exclude";

				iniFile.UpdateSection("categoryMp4Audio", category);
			}
			// mp4 Video
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Video";
				category["extension"] = "mp4";
				category["description"] = "MPEG-4 Video";
				category["ffmpeg"] = "";

				iniFile.UpdateSection("categoryMp4Video", category);
			}
			// wav
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Audio";
				category["extension"] = "wav";
				category["description"] = "Waveform Audio";
				category["ffmpeg"] = "";

				iniFile.UpdateSection("categoryWav", category);
			}
			// wmv
			{
    			CPluginIniFile::TSectionData category;

				category["type"] = "Video";
				category["extension"] = "wmv";
				category["description"] = "Windows Media Video";
				category["ffmpeg"] = "exclude";

				iniFile.UpdateSection("categoryWmv", category);
			}
		}
        s_criticalSection.Unlock();

        if (iniFile.Write())
        {
            CPluginSettings* settings = CPluginSettings::GetInstance();
            
            settings->SetValue(SETTING_CONFIG_VERSION, 4);
            settings->Write();
        }
        else
        {
            DEBUG_ERROR_LOG(iniFile.GetLastError(), PLUGIN_ERROR_CONFIG, PLUGIN_ERROR_CONFIG_CREATE_FILE, "Config::Create - Write")
        }
    }
}

bool CPluginConfig::Download(const CString& url, const CString& filename)
{
    CString tempFile = CPluginSettings::GetTempFile(TEMP_FILE_PREFIX);

    DEBUG_GENERAL("*** Downloading config:" + filename +  " (to " + tempFile + ")");

    bool bResult = !tempFile.IsEmpty();
    if (bResult)
    {
	    // if new filter urls are found download them and update the persistent data
	    HRESULT hr = ::URLDownloadToFile(NULL, url, tempFile, 0, NULL);
        if (SUCCEEDED(hr))
        {
            CPluginConfigLock lock;
            if (lock.IsLocked())
            {
                // Move the temporary file to the new text file.
                if (!::MoveFileEx(tempFile, CPluginSettings::GetDataPath(CONFIG_INI_FILE), MOVEFILE_REPLACE_EXISTING))
                {
                    DWORD dwError = ::GetLastError();

                    // Not same device? copy/delete instead
                    if (dwError == ERROR_NOT_SAME_DEVICE)
                    {
                        if (!::CopyFile(tempFile, CPluginSettings::GetDataPath(CONFIG_INI_FILE), FALSE))
                        {
                            DEBUG_ERROR_LOG(::GetLastError(), PLUGIN_ERROR_CONFIG, PLUGIN_ERROR_CONFIG_COPY_FILE, "Config::Download - CopyFile(" + filename + ")")

                            bResult = false;
                        }

                        ::DeleteFile(tempFile);
                    }
                    else
                    {
                        DEBUG_ERROR_LOG(dwError, PLUGIN_ERROR_CONFIG, PLUGIN_ERROR_CONFIG_MOVE_FILE, "Config::Download - MoveFileEx(" + filename + ")")

                        bResult = false;
                    }
                }
            }
            else
            {
                bResult = false;
            }
        }
        else
        {
            DEBUG_ERROR_LOG(hr, PLUGIN_ERROR_CONFIG, PLUGIN_ERROR_CONFIG_DOWNLOAD_FILE, "Config::Download - URLDownloadToFile(" + CString(CONFIG_INI_FILE) + ")");

            bResult = false;
        }
    }

    return bResult;
}


bool CPluginConfig::GetDownloadProperties(const CString& contentType, SDownloadFileProperties& properties) const
{
    bool isValid = false;

    s_criticalSection.Lock();
    {
        TDownloadFileProperties::const_iterator it = m_downloadFileProperties.find(contentType);
        if (it != m_downloadFileProperties.end())
        {
            properties = it->second;
            isValid = true;
		}
    }
    s_criticalSection.Unlock();

    return isValid;
}


int CPluginConfig::GenerateFilterString(TCHAR* pBuffer, SDownloadFileProperties& properties, std::vector<std::pair<CString,CString>>& filterData, bool allowConversion) const
{
	int filterIndex = 1;
	int bufferIndex = 0;

	std::map<CString,SDownloadFileCategory> filters;

    s_criticalSection.Lock();
    {
		int pos = 0;
		CString categories = properties.conversions + ";" + properties.properties.extension;
		CString category = categories.Tokenize(_T(";"), pos);
		while (pos >= 0)
		{
			for (TDownloadFileCategories::const_iterator it = m_downloadFileCategories.begin(); it != m_downloadFileCategories.end(); ++it)
			{
				if (it->second.type == category)
				{
					filters[category + " - " + it->second.description] = it->second;
				}
			}

			category = categories.Tokenize(_T(";"), pos);
		}

		int index = 1;

		for (std::map<CString,SDownloadFileCategory>::iterator it = filters.begin(); it != filters.end(); ++it)
		{
			if (allowConversion && it->second.ffmpegArgs != "exclude" || it->second.category == properties.category)
			{
				CString extension = it->second.extension;
				CString description = it->first;

				wsprintf(pBuffer + bufferIndex, L"%s (*.%s)\0", description.GetBuffer(), extension.GetBuffer());
				bufferIndex += description.GetLength() + extension.GetLength() + 6;
				wsprintf(pBuffer + bufferIndex, L".%s\0", extension.GetBuffer());
				bufferIndex += extension.GetLength() + 2;

				if (it->second.category == properties.category)
				{
					filterIndex = index;
					filterData.push_back(std::make_pair(extension, ""));
				}
				else
				{
					filterData.push_back(std::make_pair(extension, it->second.ffmpegArgs));
				}

				index++;
			}
		}
    }
    s_criticalSection.Unlock();

	pBuffer[bufferIndex] = _T('\0');

	return filterIndex;
}
