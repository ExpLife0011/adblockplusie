#include "PluginStdAfx.h"

#include "PluginWbPassThrough.h"
#include "PluginClient.h"
#include "PluginClientFactory.h"
#ifdef SUPPORT_FILTER
#include "PluginFilter.h"
#endif
#ifdef PRODUCT_DOWNLOADHELPER
#include "PluginConfig.h"
#endif
#include "PluginSettings.h"
#include "PluginClass.h"



////////////////////////////////////////////////////////////////////////////////////////
//WBPassthruSink
//Monitor and/or cancel every request and responde
//WB makes, including images, sounds, scripts, etc
////////////////////////////////////////////////////////////////////////////////////////
HRESULT WBPassthruSink::OnStart(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
		IInternetBindInfo *pOIBindInfo, DWORD grfPI, DWORD dwReserved,
		IInternetProtocol* pTargetProtocol)
{
    bool isBlocked = false;

    CString src = szUrl;
    CPluginClient::UnescapeUrl(src);
	
	SIZE_T urlLegth = wcslen(szUrl);
	if (urlLegth < IE_MAX_URL_LENGTH)
	{
		memcpy((void*)m_curUrl, (void*)szUrl, urlLegth * 2);
		m_curUrl[urlLegth] = '\0';
	} 
	else 
	{
		memcpy((void*)m_curUrl, (void*)szUrl, IE_MAX_URL_LENGTH * 2);
		m_curUrl[urlLegth] = '\0';
	}
	int contentType = CFilter::contentTypeAny;

#ifdef SUPPORT_FILTER

	CPluginTab* tab = CPluginClass::GetTab(::GetCurrentThreadId());
    CPluginClient* client = CPluginClient::GetInstance();
    if (tab && client)
	{
		CString documentUrl = tab->GetDocumentUrl();

		// Page is identical to document => don't block
		if (documentUrl == src)
		{
			// fall through
		}
		else if (CPluginSettings::GetInstance()->IsPluginEnabled() && !client->IsUrlWhiteListed(documentUrl))
		{
	        CString domain = tab->GetDocumentDomain();

			contentType = CFilter::contentTypeAny;

#ifdef SUPPORT_FRAME_CACHING
            if (tab->IsFrameCached(src))
            {
                contentType = CFilter::contentTypeSubdocument;
            }
            else
#endif // SUPPORT_FRAME_CACHING
			{
                CString srcExt = src;

			    int pos = 0;
			    if ((pos = src.Find('?')) > 0)
			    {
				    srcExt = src.Left(pos);
			    }

			    CString ext = srcExt.Right(4);

			    if (ext == L".jpg" || ext == L".gif" || ext == L".png")
			    {
				    contentType = CFilter::contentTypeImage;
			    }
			    else if (ext == L".css")
			    {
				    contentType = CFilter::contentTypeStyleSheet;
			    }
			    else if (ext.Right(3) == L".js")
			    {
				    contentType = CFilter::contentTypeScript;
			    }
			    else if (ext == L".xml")
			    {
				    contentType = CFilter::contentTypeXmlHttpRequest;
			    }
			    else if (ext == L".swf")
			    {
				    contentType = CFilter::contentTypeObjectSubrequest;
			    }
			    else if (ext == L".jsp" || ext == L".php")
			    {
				    contentType = CFilter::contentTypeSubdocument;
			    }
			    else
			    {
				    contentType = CFilter::contentTypeAny & ~CFilter::contentTypeSubdocument;
			    }
	        }

			if (client->ShouldBlock(src, contentType, domain, true))
			{
				DEBUG_BLOCKER("Blocker::Blocking Http-request:" + src);

                isBlocked = true;
			}
#ifdef ENABLE_DEBUG_RESULT_IGNORED
			else
			{
			    CString type;

        		if (contentType == CFilter::contentTypeDocument) type = "doc";
	            else if (contentType == CFilter::contentTypeObject) type = "object";
	            else if (contentType == CFilter::contentTypeImage) type = "img";
	            else if (contentType == CFilter::contentTypeScript) type = "script";
	            else if (contentType == CFilter::contentTypeOther) type = "other";
	            else if (contentType == CFilter::contentTypeUnknown) type = "?";
	            else if (contentType == CFilter::contentTypeSubdocument) type = "iframe";
	            else if (contentType == CFilter::contentTypeStyleSheet) type = "css";
	            else type = "???";

                CPluginDebug::DebugResultIgnoring(type, src);
            }
#endif // ENABLE_DEBUG_RESULT_IGNORED
		}

        if (!isBlocked)
        {
            DEBUG_BLOCKER("Blocker::Ignoring Http-request:" + src)
        }
	}

#endif // SUPPORT_FILTER

	//Fixes the iframe back button issue
	if ((contentType == CFilter::contentTypeSubdocument) && (isBlocked)) 
	{
		memcpy(m_curUrl, L"http://zaviriuh.shoniko.operaunite.com/webserver/content/test.htm", wcslen(L"http://zaviriuh.shoniko.operaunite.com/webserver/content/test.htm") * 2);
		m_curUrl[wcslen(L"http://zaviriuh.shoniko.operaunite.com/webserver/content/test.htm")] = '\0';
		return S_FALSE;
	}
	return isBlocked ? S_FALSE : BaseClass::OnStart(szUrl, pOIProtSink, pOIBindInfo, grfPI, dwReserved, pTargetProtocol);
}

STDMETHODIMP WBPassthruSink::Switch(
	/* [in] */ PROTOCOLDATA *pProtocolData)
{
	ATLASSERT(m_spInternetProtocolSink != 0);

/*
From Igor Tandetnik "itandetnik@mvps.org"
"
Beware multithreading. URLMon has this nasty habit of spinning worker 
threads, not even bothering to initialize COM on them, and calling APP 
methods on those threads. If you try to raise COM events directly from 
such a thread, bad things happen (random crashes, events being lost). 
You are only guaranteed to be on the main STA thread in two cases. 
First, in methods of interfaces that were obtained with 
IServiceProvider, such as IHttpNegotiage::BeginningTransaction or 
IAuthenticate::Authenticate. Second, you can call 
IInternetProtocolSink::Switch with PD_FORCE_SWITCH flag in 
PROTOCOLDATA::grfFlags, eventually URLMon will turn around and call 
IInternetProtocol::Continue on the main thread. 

Or, if you happen to have a window handy that was created on the main 
thread, you can post yourself a message.
"
*/
	return m_spInternetProtocolSink ? m_spInternetProtocolSink->Switch(pProtocolData) : E_UNEXPECTED;
}


STDMETHODIMP WBPassthruSink::BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders, DWORD dwReserved, LPWSTR *pszAdditionalHeaders)
{
	if (pszAdditionalHeaders)
	{
		*pszAdditionalHeaders = 0;
	}

	CComPtr<IHttpNegotiate> spHttpNegotiate;
	QueryServiceFromClient(&spHttpNegotiate);

	return spHttpNegotiate ? spHttpNegotiate->BeginningTransaction(szURL, szHeaders,dwReserved, pszAdditionalHeaders) : S_OK;
}

STDMETHODIMP WBPassthruSink::OnResponse(DWORD dwResponseCode, LPCWSTR szResponseHeaders, LPCWSTR szRequestHeaders, LPWSTR *pszAdditionalRequestHeaders)
{
	if (pszAdditionalRequestHeaders)
	{
		*pszAdditionalRequestHeaders = 0;
	}

	CComPtr<IHttpNegotiate> spHttpNegotiate;
	QueryServiceFromClient(&spHttpNegotiate);

#ifdef PRODUCT_DOWNLOADHELPER

	CPluginTab* tab = CPluginClass::GetTab(::GetCurrentThreadId());
    CPluginClient* client = CPluginClient::GetInstance();
    if (tab && client)
    {
		CPluginConfig* config = CPluginConfig::GetInstance();
        
        CString contentType = szResponseHeaders;

        int pos = contentType.Find(L"Content-Type: ");
        if (pos >= 0)
        {
            contentType = contentType.Mid(pos + 14);
            
            pos = contentType.FindOneOf(L"; \n\r");
            if (pos > 0)
            {				
				bool isDownloadFile = false;
				SDownloadFileProperties downloadFileProperties;

				contentType = contentType.Left(pos);
				if (config->GetDownloadProperties(contentType, downloadFileProperties))
                {
					isDownloadFile = true;
                }
				// Special hacks
				else if (contentType == "text/plain")
				{
					CString domain = tab->GetDocumentDomain();
					if (domain == "fragstein.org" && m_url.Find(L".flv") == m_url.GetLength() - 4)
					{
						isDownloadFile = config->GetDownloadProperties("video/x-flv", downloadFileProperties);
					}
					else if (domain == "metacafe.com" && m_url.Find(L"http://akvideos.metacafe.com/ItemFiles/") == 0)
					{
						isDownloadFile = config->GetDownloadProperties("video/x-flv", downloadFileProperties);
					}
				}

				if (isDownloadFile)
				{
					// Find length
					CStringA contentLength = szResponseHeaders;

					int posLength = contentLength.Find("Content-Length: ");
					if (posLength > 0)
					{
						contentLength = contentLength.Mid(posLength + 16);
			            
						posLength = contentLength.FindOneOf("; \n\r");
						if (posLength > 0)
						{
							int fileSize = atoi(contentLength.Left(posLength).GetBuffer());
							if (fileSize > 0)
							{
								tab->AddDownloadFile(m_url, fileSize, downloadFileProperties);
							}
						}
					}
				}
            }
        }
		else
		{
		}
    }

#endif // PRODUCT_DOWNLOADHELPER

	return spHttpNegotiate ? spHttpNegotiate->OnResponse(dwResponseCode, szResponseHeaders, szRequestHeaders, pszAdditionalRequestHeaders) : S_OK;
}

STDMETHODIMP WBPassthruSink::ReportProgress(ULONG ulStatusCode, LPCWSTR szStatusText)
{
	return m_spInternetProtocolSink ? m_spInternetProtocolSink->ReportProgress(ulStatusCode, szStatusText) : S_OK;
}