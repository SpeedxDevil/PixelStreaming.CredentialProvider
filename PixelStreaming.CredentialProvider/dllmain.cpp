// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "PixelStreamingCredentialProvider.h" // For CLSID_PixelStreamingCredentialProvider
#include "ClassFactory.h" // For CClassFactory
#include <strsafe.h> // For StringCchPrintf

// Global module instance handle
HINSTANCE g_hModule = nullptr;
LONG g_cRefModule = 0; // Global server lock count / outstanding object count

// Registry helper functions
HRESULT CreateRegKeyAndSetValue(HKEY hKeyRoot, LPCWSTR pszSubKey, LPCWSTR pszValueName, LPCWSTR pszValue);
HRESULT SetRegValue(HKEY hKeyRoot, LPCWSTR pszSubKey, LPCWSTR pszValueName, LPCWSTR pszValue); // Assumes subkey exists
HRESULT DeleteRegKey(HKEY hKeyRoot, LPCWSTR pszSubKey, LPCWSTR pszKeyName);


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule); // Optimization for non-multithreaded DLLs
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        g_hModule = nullptr;
        break;
    }
    return TRUE;
}

// Standard COM DLL exports
STDAPI DllCanUnloadNow()
{
    // return (g_cRefModule == 0) ? S_OK : S_FALSE;
    // For credential providers, it's often simpler to always return S_FALSE
    // as the lifetime is managed by the OS logon/credential UI processes.
    // Or, implement proper module reference counting if other COM objects are hosted.
    return S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
    if (ppv == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppv = nullptr;

    if (IsEqualCLSID(rclsid, CLSID_PixelStreamingCredentialProvider))
    {
        CClassFactory* pFactory = new (std::nothrow) CClassFactory(rclsid);
        if (pFactory == nullptr)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pFactory->QueryInterface(riid, ppv);
            pFactory->Release(); // QueryInterface AddRef'd if successful
        }
    }
    return hr;
}

STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;
    WCHAR szCLSID[MAX_PATH];
    WCHAR szModulePath[MAX_PATH];

    // Get the CLSID as a string
    if (!StringFromGUID2(CLSID_PixelStreamingCredentialProvider, szCLSID, ARRAYSIZE(szCLSID)))
    {
        return E_UNEXPECTED;
    }

    // Get the module path
    if (GetModuleFileNameW(g_hModule, szModulePath, ARRAYSIZE(szModulePath)) == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Create the CLSID key: HKEY_CLASSES_ROOT\CLSID\{CLSID_PixelStreamingCredentialProvider}
    WCHAR szKeyCLSID[MAX_PATH];
    hr = StringCchPrintfW(szKeyCLSID, ARRAYSIZE(szKeyCLSID), L"CLSID\\%s", szCLSID);
    if (FAILED(hr)) return hr;

    hr = CreateRegKeyAndSetValue(HKEY_CLASSES_ROOT, szKeyCLSID, nullptr, L"PixelStreaming Credential Provider");
    if (FAILED(hr)) return hr;

    // Create the InprocServer32 key: HKEY_CLASSES_ROOT\CLSID\{CLSID_PixelStreamingCredentialProvider}\InprocServer32
    WCHAR szKeyInproc[MAX_PATH];
    hr = StringCchPrintfW(szKeyInproc, ARRAYSIZE(szKeyInproc), L"%s\\InprocServer32", szKeyCLSID);
    if (FAILED(hr)) return hr;

    hr = CreateRegKeyAndSetValue(HKEY_CLASSES_ROOT, szKeyInproc, nullptr, szModulePath);
    if (FAILED(hr)) return hr;

    // Set the threading model: HKEY_CLASSES_ROOT\CLSID\{CLSID_PixelStreamingCredentialProvider}\InprocServer32 "ThreadingModel"="Apartment"
    hr = SetRegValue(HKEY_CLASSES_ROOT, szKeyInproc, L"ThreadingModel", L"Apartment");
    if (FAILED(hr)) return hr;

    // Register the credential provider itself under HKEY_LOCAL_MACHINE
    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{CLSID_PixelStreamingCredentialProvider}
    WCHAR szKeyCredProvider[MAX_PATH];
    hr = StringCchPrintfW(szKeyCredProvider, ARRAYSIZE(szKeyCredProvider), L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\%s", szCLSID);
    if (FAILED(hr)) return hr;

    hr = CreateRegKeyAndSetValue(HKEY_LOCAL_MACHINE, szKeyCredProvider, nullptr, L"PixelStreaming Credential Provider");
    // If this key creation fails due to permissions, the registration will fail.
    // This typically requires administrator privileges.

    return hr;
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;
    WCHAR szCLSID[MAX_PATH];
    WCHAR szKeyPath[MAX_PATH]; // Used for various key paths

    if (!StringFromGUID2(CLSID_PixelStreamingCredentialProvider, szCLSID, ARRAYSIZE(szCLSID)))
    {
        return E_UNEXPECTED;
    }

    // Delete HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{CLSID_PixelStreamingCredentialProvider}
    hr = StringCchPrintfW(szKeyPath, ARRAYSIZE(szKeyPath), L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\%s", szCLSID);
    if (SUCCEEDED(hr))
    {
        // DeleteRegKey will try to delete the key itself.
        // RegDeleteTree is more robust for deleting a key and all its subkeys/values.
        // For now, a simple recursive delete might be what's intended by DeleteRegKey.
        // If DeleteRegKey is simple, it might fail if subkeys exist.
        // However, our provider doesn't create subkeys under this one.
        LSTATUS ls = RegDeleteKeyW(HKEY_LOCAL_MACHINE, szKeyPath);
        if (ls != ERROR_SUCCESS && ls != ERROR_FILE_NOT_FOUND) hr = HRESULT_FROM_WIN32(ls);
    }


    // Delete HKEY_CLASSES_ROOT\CLSID\{CLSID_PixelStreamingCredentialProvider}\InprocServer32
    // It's safer to delete InprocServer32 first, then the CLSID key.
    hr = StringCchPrintfW(szKeyPath, ARRAYSIZE(szKeyPath), L"CLSID\\%s\\InprocServer32", szCLSID);
    if (SUCCEEDED(hr))
    {
        LSTATUS ls = RegDeleteKeyW(HKEY_CLASSES_ROOT, szKeyPath);
         if (ls != ERROR_SUCCESS && ls != ERROR_FILE_NOT_FOUND && SUCCEEDED(hr)) hr = HRESULT_FROM_WIN32(ls);
    }

    // Delete HKEY_CLASSES_ROOT\CLSID\{CLSID_PixelStreamingCredentialProvider}
    hr = StringCchPrintfW(szKeyPath, ARRAYSIZE(szKeyPath), L"CLSID\\%s", szCLSID);
    if (SUCCEEDED(hr))
    {
        LSTATUS ls = RegDeleteKeyW(HKEY_CLASSES_ROOT, szKeyPath);
        if (ls != ERROR_SUCCESS && ls != ERROR_FILE_NOT_FOUND && SUCCEEDED(hr)) hr = HRESULT_FROM_WIN32(ls);
    }

    // A more robust unregistration would use RegDeleteTree for the CLSID key.
    // For example:
    // StringCchPrintfW(szKeyPath, ARRAYSIZE(szKeyPath), L"CLSID\\%s", szCLSID);
    // RegDeleteTreeW(HKEY_CLASSES_ROOT, szKeyPath);
    // StringCchPrintfW(szKeyPath, ARRAYSIZE(szKeyPath), L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\%s", szCLSID);
    // RegDeleteTreeW(HKEY_LOCAL_MACHINE, szKeyPath);


    return hr; // Return the last error encountered, or S_OK if all good.
}


// Helper function implementations
HRESULT CreateRegKeyAndSetValue(HKEY hKeyRoot, LPCWSTR pszSubKey, LPCWSTR pszValueName, LPCWSTR pszValue)
{
    HKEY hKey;
    LSTATUS status = RegCreateKeyExW(hKeyRoot, pszSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    if (status != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(status);
    }

    if (pszValue != nullptr) // If pszValue is null, we just create the key
    {
        status = RegSetValueExW(hKey, pszValueName, 0, REG_SZ, (const BYTE*)pszValue, (DWORD)((wcslen(pszValue) + 1) * sizeof(WCHAR)));
    }

    RegCloseKey(hKey);
    return (status == ERROR_SUCCESS) ? S_OK : HRESULT_FROM_WIN32(status);
}

HRESULT SetRegValue(HKEY hKeyRoot, LPCWSTR pszSubKey, LPCWSTR pszValueName, LPCWSTR pszValue)
{
    HKEY hKey;
    LSTATUS status = RegOpenKeyExW(hKeyRoot, pszSubKey, 0, KEY_WRITE, &hKey);
    if (status != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(status);
    }
    status = RegSetValueExW(hKey, pszValueName, 0, REG_SZ, (const BYTE*)pszValue, (DWORD)((wcslen(pszValue) + 1) * sizeof(WCHAR)));
    RegCloseKey(hKey);
    return (status == ERROR_SUCCESS) ? S_OK : HRESULT_FROM_WIN32(status);
}

// Note: DeleteRegKey is not implemented here, DllUnregisterServer uses RegDeleteKeyW directly.
// A more robust DeleteRegKey might recursively delete subkeys if needed.
// For this provider, direct deletion is likely sufficient.
