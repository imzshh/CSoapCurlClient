#include <atlsoap.h>

class CMemoryStream :
    public IStreamImpl
{
public:
    CMemoryStream()
        : dwPosition(0)
        , pData(0)
        , dwSize(0)
        , dwCapability(4096)
    {
        this->pData = new BYTE[dwCapability];
    }

    ~CMemoryStream()
    {
        delete[] pData;
    }

private:
    DWORD dwPosition;
    BYTE* pData;
    DWORD dwSize;
    DWORD dwCapability;

public:
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == NULL)
		{
			return E_POINTER;
		}

		*ppv = NULL;

		if (InlineIsEqualGUID(riid, IID_IUnknown) ||
			InlineIsEqualGUID(riid, IID_IStream) ||
			InlineIsEqualGUID(riid, IID_ISequentialStream))
		{
			*ppv = static_cast<IStream *>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

private:
    void CheckCapability(ULONG cb)
    {
        if(this->dwSize + cb > this->dwCapability)
        {
            this->ExpandCapability();
        }
    }

    void ExpandCapability()
    {
        BYTE* pOldData = pData;
        DWORD newCapability = this->dwCapability * 1.5;
        BYTE* newData = new BYTE[newCapability];
        ::memcpy(newData, pOldData, this->dwSize);
        this->dwCapability = newCapability;
        delete[] pOldData;
    }

public:
    void Clear()
    {
        delete[] pData;
        dwSize = 0;
        dwPosition = 0;
        dwCapability = 4096;
        this->pData = new BYTE[dwCapability];
    }

	void SetPosition(DWORD position)
    {
        this->dwPosition = position;
    }

	HRESULT __stdcall Read(void * pDest, ULONG nMaxLen, ULONG * pnRead)
	{
		ATLENSURE( pDest != NULL );

        if(dwPosition >= dwSize)
        {
            pnRead = 0;
		    return S_OK;
        }

        if(nMaxLen + dwPosition > dwSize)
        {
            *pnRead = dwSize - dwPosition;
        }

        ::memcpy(pDest, pData + dwPosition, *pnRead);
        dwPosition += *pnRead;

		return S_OK;
	}

    HRESULT __stdcall Write(const void * pv, ULONG cb, ULONG * pcbWritten)
	{
		ATLENSURE( pv != NULL );

        this->CheckCapability(cb);

        ::memcpy(pData + this->dwPosition, pv, cb);
        this->dwPosition += cb;
        this->dwSize = this->dwPosition;

        *pcbWritten = cb;

		return E_FAIL;
	}
};
