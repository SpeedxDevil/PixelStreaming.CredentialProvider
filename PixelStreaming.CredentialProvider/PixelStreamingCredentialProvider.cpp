// PixelStreamingCredentialProvider.cpp
#include <initguid.h> // Must be before other headers that use the GUID
#include "pch.h"
#include "PixelStreamingCredentialProvider.h"
#include "PixelStreamingCredential.h" // Need this for creating credential object
#include <strsafe.h> // For StringCchCopyW
#include <new> // for std::nothrow

// Field IDs for our two fields.
#define USERNAME_FIELD_ID 0
#define PASSWORD_FIELD_ID 1

// The labels for our fields.
static const PWSTR s_rgFieldLabels[] =
{
    L"Username",
    L"Password",
};

// Descriptions of the fields.
const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR CPixelStreamingCredentialProvider::s_rgFieldDescriptors[] =
{
    { USERNAME_FIELD_ID, CPFT_EDIT_TEXT, s_rgFieldLabels[USERNAME_FIELD_ID], { 0 } /* guidFieldType not used for CPFT_EDIT_TEXT */ },
    { PASSWORD_FIELD_ID, CPFT_PASSWORD_TEXT, s_rgFieldLabels[PASSWORD_FIELD_ID], { 0 } /* guidFieldType not used for CPFT_PASSWORD_TEXT */ },
};

const DWORD CPixelStreamingCredentialProvider::s_dwFieldDescriptorCount = ARRAYSIZE(s_rgFieldDescriptors);

// Helper function to create and initialize a CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR.
HRESULT FieldDescriptorCoAllocCopy(
    const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    HRESULT hr = E_INVALIDARG;
    if (ppcpfd != nullptr)
    {
        *ppcpfd = nullptr;
        CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd =
            static_cast<CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*>(CoTaskMemAlloc(sizeof(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR)));
        if (pcpfd != nullptr)
        {
            pcpfd->dwFieldID = rcpfd.dwFieldID;
            pcpfd->cpft = rcpfd.cpft;
            pcpfd->guidFieldType = rcpfd.guidFieldType;

            // CoTaskMemAlloc and copy the label.
            hr = SHStrDupW(rcpfd.pszLabel, &(pcpfd->pszLabel));
            if (SUCCEEDED(hr))
            {
                *ppcpfd = pcpfd;
            }
            else
            {
                CoTaskMemFree(pcpfd);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

// Helper function to free a CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR.
void FieldDescriptorFree(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd)
{
    if (pcpfd != nullptr)
    {
        CoTaskMemFree(pcpfd->pszLabel);
        CoTaskMemFree(pcpfd);
    }
}


CPixelStreamingCredentialProvider::CPixelStreamingCredentialProvider() :
    _cRef(1),
    _cpus(CPUS_INVALID),
    _pcpe(nullptr),
    _upAdviseContext(0)
{
    _cpcs.rgbSerialization = nullptr;
    _cpcs.ulSerializationBufferSize = 0;
    _cpcs.clsidCredentialProvider = GUID_NULL;
    _cpcs.cpus = CPUS_INVALID;
}

CPixelStreamingCredentialProvider::~CPixelStreamingCredentialProvider()
{
    if (_pcpe != nullptr)
    {
        _pcpe->Release();
        _pcpe = nullptr;
    }
    if (_cpcs.rgbSerialization != nullptr)
    {
        CoTaskMemFree(_cpcs.rgbSerialization);
        _cpcs.rgbSerialization = nullptr;
    }
}

// IUnknown
IFACEMETHODIMP CPixelStreamingCredentialProvider::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CPixelStreamingCredentialProvider, ICredentialProvider),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CPixelStreamingCredentialProvider::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) CPixelStreamingCredentialProvider::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

// ICredentialProvider
IFACEMETHODIMP CPixelStreamingCredentialProvider::SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags)
{
    _cpus = cpus;
    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredentialProvider::SetSerialization(const CREDENTIAL_PROVIDER_SERIALIZATION* pcpcs)
{
    if (pcpcs == nullptr)
    {
        return E_INVALIDARG;
    }

    if (pcpcs->rgbSerialization != nullptr && pcpcs->ulSerializationBufferSize > 0)
    {
        // Deep copy the serialization data
        _cpcs.rgbSerialization = (byte*)CoTaskMemAlloc(pcpcs->ulSerializationBufferSize);
        if (_cpcs.rgbSerialization == nullptr)
        {
            return E_OUTOFMEMORY;
        }
        memcpy(_cpcs.rgbSerialization, pcpcs->rgbSerialization, pcpcs->ulSerializationBufferSize);
        _cpcs.ulSerializationBufferSize = pcpcs->ulSerializationBufferSize;
        _cpcs.clsidCredentialProvider = pcpcs->clsidCredentialProvider; // This should be our CLSID
        _cpcs.cpus = pcpcs->cpus;
    }
    else
    {
        // Clear any existing serialization data if null is passed
        if (_cpcs.rgbSerialization != nullptr)
        {
            CoTaskMemFree(_cpcs.rgbSerialization);
            _cpcs.rgbSerialization = nullptr;
        }
        _cpcs.ulSerializationBufferSize = 0;
        _cpcs.clsidCredentialProvider = GUID_NULL;
        _cpcs.cpus = CPUS_INVALID;
    }

    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredentialProvider::Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext)
{
    if (pcpe != nullptr)
    {
        _pcpe = pcpe;
        _pcpe->AddRef();
        _upAdviseContext = upAdviseContext;
        return S_OK;
    }
    return E_INVALIDARG;
}

IFACEMETHODIMP CPixelStreamingCredentialProvider::UnAdvise()
{
    if (_pcpe != nullptr)
    {
        _pcpe->Release();
        _pcpe = nullptr;
        _upAdviseContext = 0;
    }
    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredentialProvider::GetFieldDescriptorCount(DWORD* pdwCount)
{
    if (pdwCount == nullptr)
    {
        return E_INVALIDARG;
    }
    *pdwCount = s_dwFieldDescriptorCount;
    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredentialProvider::GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    HRESULT hr;
    if (ppcpfd == nullptr)
    {
        hr = E_INVALIDARG;
    }
    else if (dwIndex >= s_dwFieldDescriptorCount)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = FieldDescriptorCoAllocCopy(s_rgFieldDescriptors[dwIndex], ppcpfd);
    }
    return hr;
}

IFACEMETHODIMP CPixelStreamingCredentialProvider::GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault)
{
    if (pdwCount == nullptr || pdwDefault == nullptr || pbAutoLogonWithDefault == nullptr)
    {
        return E_INVALIDARG;
    }

    // For now, we will always provide one credential.
    // This credential will not be the default, and auto-logon will not be enabled for it.
    *pdwCount = 1;
    *pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT; // Or 0 if we want the first to be default by index
    *pbAutoLogonWithDefault = FALSE;

    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredentialProvider::GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
{
    if (ppcpc == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppcpc = nullptr;

    // We only have one credential type. So, dwIndex must be 0.
    if (dwIndex != 0)
    {
        return E_INVALIDARG;
    }

    CPixelStreamingCredential* pCredential = new (std::nothrow) CPixelStreamingCredential();
    if (pCredential == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // Example: Initialize with current usage scenario and potentially prefill username from serialization
    PWSTR pszPrefillUsername = nullptr;
    // TODO: Logic to extract username from _cpcs if available and appropriate for the _cpus
    // For example, if _cpcs.rgbSerialization contains a username from a previous session.
    // This is a simplified placeholder. A robust implementation would deserialize _cpcs properly.
    // if (_cpcs.rgbSerialization && _cpcs.ulSerializationBufferSize > 0 && _cpus == CPUS_LOGON) {
    //    // Assuming _cpcs.rgbSerialization is just a PWSTR username for this example
    //    // THIS IS NOT A SECURE OR ROBUST WAY TO HANDLE SERIALIZATION - FOR ILLUSTRATION ONLY
    //    SHStrDupW(reinterpret_cast<PWSTR>(_cpcs.rgbSerialization), &pszPrefillUsername);
    // }


    HRESULT hr = pCredential->Initialize(_cpus, pszPrefillUsername);
    CoTaskMemFree(pszPrefillUsername); // Initialize would have made its own copy if needed

    if (FAILED(hr))
    {
        pCredential->Release();
        return hr;
    }

    hr = pCredential->QueryInterface(IID_PPV_ARGS(ppcpc));
    if (FAILED(hr))
    {
        pCredential->Release();
    }
    // pCredential->Release(); // QueryInterface AddRef's, so we release our local ref.
                             // Actually, the object starts with ref 1, QI adds one, so the caller gets a ref of 2.
                             // The caller is responsible for Releasing. We should release our initial ref.
                             // No, standard COM practice: if new object created, it has ref 1.
                             // QI gives out a new ref (caller owns it), so we still own our original ref.
                             // We must release our original ref.
    pCredential->Release();


    return hr;
}
