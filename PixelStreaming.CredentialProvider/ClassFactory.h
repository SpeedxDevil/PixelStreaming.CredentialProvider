// ClassFactory.h
#pragma once

#include <unknwn.h> // For IClassFactory
#include <windows.h>

class CClassFactory : public IClassFactory
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override;
    IFACEMETHODIMP LockServer(BOOL fLock) override;

public:
    CClassFactory(REFCLSID clsid); // Constructor takes the CLSID it will create

private:
    ~CClassFactory(); // Prevent direct deletion

    LONG _cRef;
    CLSID _clsid; // CLSID of the object this factory creates
};
