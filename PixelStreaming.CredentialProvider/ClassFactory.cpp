// ClassFactory.cpp
#include "pch.h"
#include "ClassFactory.h"
#include "PixelStreamingCredentialProvider.h" // For CPixelStreamingCredentialProvider and its CLSID
#include <new> // for std::nothrow

CClassFactory::CClassFactory(REFCLSID clsid) : _cRef(1), _clsid(clsid)
{
    // Module ref count can be incremented here if needed, e.g. g_cRefModule++;
}

CClassFactory::~CClassFactory()
{
    // Module ref count can be decremented here if needed, e.g. g_cRefModule--;
}

// IUnknown
IFACEMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CClassFactory, IClassFactory),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) CClassFactory::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

// IClassFactory
IFACEMETHODIMP CClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
    HRESULT hr;
    if (pUnkOuter != nullptr)
    {
        // No aggregation supported
        hr = CLASS_E_NOAGGREGATION;
    }
    else
    {
        if (IsEqualCLSID(_clsid, CLSID_PixelStreamingCredentialProvider))
        {
            CPixelStreamingCredentialProvider* pProvider = new (std::nothrow) CPixelStreamingCredentialProvider();
            if (pProvider == nullptr)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                hr = pProvider->QueryInterface(riid, ppv);
                pProvider->Release(); // Release initial ref, QueryInterface AddRef'd if successful
            }
        }
        else
        {
            hr = CLASS_E_CLASSNOTAVAILABLE; // Should not happen if constructor got the right CLSID
        }
    }
    return hr;
}

IFACEMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    // if (fLock)
    // {
    //     g_cRefModule++;
    // }
    // else
    // {
    //     g_cRefModule--;
    // }
    return S_OK; // Module lifetime management can be added here
}
