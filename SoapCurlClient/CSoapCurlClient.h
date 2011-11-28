#include <curl/curl.h>
#include "CMemoryStream.h"

class CSoapCurlClient
{
private:
    CUrl m_url;
    LPCTSTR m_szURL;
    CWriteStreamOnCString m_writeStream;
    DWORD m_dwTimeout;
    SOAPCLIENT_ERROR m_errorState;

    CMemoryStream* m_pDataStream;

    CURL *m_pCurl;
private:
    static size_t ReceiveCurlData(char *data, int size, int nmemb, CMemoryStream* pStream)
    {
        ULONG sizes = size * nmemb;
        pStream->Write(data, sizes, &sizes);
        return sizes;
    }

protected:
	virtual HRESULT GetClientReader(ISAXXMLReader **pReader)
	{
		if (pReader == NULL)
		{
			return E_POINTER;
		}
		*pReader = NULL;

		CComPtr<ISAXXMLReader> spReader;
		HRESULT hr = spReader.CoCreateInstance(ATLS_SAXXMLREADER_CLSID, NULL, CLSCTX_INPROC_SERVER);
		if (SUCCEEDED(hr))
		{
			*pReader = spReader.Detach();
		}
		return hr;
	}
public:
    CSoapCurlClient(LPCTSTR szUrl)
        :m_dwTimeout(0), m_errorState(SOAPCLIENT_SUCCESS)
    {
        this->SetUrl(szUrl);
        this->m_pDataStream = new CMemoryStream();
    }

    virtual ~CSoapCurlClient()
    {
        CleanupClient();
        delete this->m_pDataStream;
    }

    void SetUrl(LPCTSTR szUrl)
    {
        this->m_szURL = szUrl;
        this->m_url.CrackUrl(szUrl);
    }

    void SetClientError(SOAPCLIENT_ERROR errorState)
    {
        m_errorState = errorState;
    }

    void CleanupClient()
    {
        this->m_writeStream.Cleanup();
        this->m_pDataStream->Clear();
        SetClientError(SOAPCLIENT_SUCCESS);
    }

    IWriteStream * GetWriteStream()
    {
        return &m_writeStream;
    }

    HRESULT SendRequest(LPCTSTR szAction)
    {
        HRESULT hr = S_OK;

        CURL *curl;
        CURLcode res;

        this->m_pCurl = curl = curl_easy_init();

        if(curl)
        {
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, (LPCSTR)CStringA(this->m_szURL));
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            // set header
            struct curl_slist *headerlist=NULL;
            CStringA strHeaderContentLen;
            strHeaderContentLen.Format("Content-Length: %d", m_writeStream.m_str.GetLength());
            headerlist = curl_slist_append(headerlist, (LPCSTR)strHeaderContentLen);
            CStringA strHost;
            strHost.Format("Host: %s", this->m_url.GetHostName());
            headerlist = curl_slist_append(headerlist, (LPCSTR)strHost);
            headerlist = curl_slist_append(headerlist, "Content-Type: text/xml; charset=utf-8");
            headerlist = curl_slist_append(headerlist, "Accept: text/xml");
            headerlist = curl_slist_append(headerlist, (LPCSTR)CStringA(szAction).Trim());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this->m_pDataStream);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CSoapCurlClient::ReceiveCurlData);

            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (LPCSTR)m_writeStream.m_str);

            res = curl_easy_perform(curl);

            if(res != CURLE_OK)
            {
                hr = SOAPCLIENT_SEND_ERROR;
            }

            int nResponseCode;
            res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &nResponseCode);
            if(res != CURLE_OK || nResponseCode != 200)
            {
                hr = SOAPCLIENT_SEND_ERROR;
            }

            curl_easy_cleanup(curl);
            curl_slist_free_all (headerlist);
        }

        return hr;
    }

    HRESULT GetReadStream(IStream **ppStream)
    {
        HRESULT hr = S_OK;

        this->m_pDataStream->SetPosition(0);
        *ppStream = this->m_pDataStream;

        return hr;
    }
};
