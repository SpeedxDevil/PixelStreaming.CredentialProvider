// helpers.h
#pragma once

#include <windows.h>
#include <credentialprovider.h> // For CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR and other types
#include <ntsecapi.h> // For UNICODE_STRING
#include <Shlwapi.h> // For SHStrDupW

// Forward declarations if necessary (though for these simple helpers, maybe not)

// Safe coTaskMemAlloc for strings
HRESULT CoTaskStringAlloc(PCWSTR pszSource, PWSTR* ppszDest);

// General string duplication using CoTaskMemAlloc
HRESULT CoTaskDupString(PCWSTR pszSource, PWSTR* ppszDest);

// Field descriptor helpers (already in PixelStreamingCredentialProvider.h/cpp but could be centralized if more classes use them)
// HRESULT FieldDescriptorCoAllocCopy(...)
// void FieldDescriptorFree(...)

// Securely zero memory
void SecureZeroMemoryEx(void* ptr, SIZE_T cnt);

// Convert a BSTR to a CoTaskMemAllocated PWSTR.
HRESULT BstrToCoTaskMemStr(BSTR bstr, PWSTR* ppszOut);

// Convert a PWSTR to a BSTR.
HRESULT PwstrToBstr(PCWSTR pszIn, BSTR* pbsOut);


// Definitions for common field IDs (can be shared between provider and credential)
#define USERNAME_FIELD_ID 0
#define PASSWORD_FIELD_ID 1
#define SUBMIT_BUTTON_ID 2 // Example, if we add a submit button

// Structure to pack username and password for serialization
typedef struct _USERNAME_PASSWORD_CREDENTIAL
{
    PWSTR pszUsername;
    PWSTR pszPassword;
} USERNAME_PASSWORD_CREDENTIAL;

// Helper to pack username and password into a UNICODE_STRING for LsaLogonUser.
// Caller must free unicodeString->Buffer with HeapFree(GetProcessHeap(), 0, unicodeString->Buffer)
BOOL PackUnicodeString(PCWSTR psz, UNICODE_STRING* punicodeString);

// Helper to get the password and domain/username as Unicode strings.
// Callers must free the returned strings with CoTaskMemFree.
HRESULT ProtectIfNecessaryAndCopyPassword(
    PCWSTR pszPassword,
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    PWSTR* ppszProtectedPassword);

// Gets the SID for a user.
// Caller must free *ppsid with CoTaskMemFree.
HRESULT GetSidFromToken(HANDLE hToken, PSID* ppsid);

// Gets the token for a user.
// Caller must close hToken with CloseHandle.
HRESULT GetUserToken(PCWSTR pszUsername, PCWSTR pszPassword, PCWSTR pszDomain, HANDLE* phToken);

// SID to string SID.
// Caller must free *ppszStringSid with LocalFree.
BOOL ConvertSidToStringSidPreserveErr(PSID pSid, LPWSTR* ppszStringSid);

// String SID to SID.
// Caller must free *ppSid with LocalFree.
BOOL ConvertStringSidToSidPreserveErr(LPCWSTR pszStringSid, PSID* ppSid);


// From existing sample code, useful for Kerberos S4U logon
#define KERB_LOGON_SUBMIT_TYPE L"KerbLogon"
#define KERB_INTERACTIVE_UNLOCK_LOGON L"KerbInteractiveUnlockLogon"

#define NEGOTIATE_LOGON_SUBMIT_TYPE L"NegotiateLogon"
#define NEGOTIATE_INTERACTIVE_UNLOCK_LOGON L"NegotiateInteractiveUnlockLogon"

#define NTLM_LOGON_SUBMIT_TYPE L"NtlmLogon"
#define NTLM_INTERACTIVE_UNLOCK_LOGON L"NtlmInteractiveUnlockLogon"


// Used to pack a credential for GetSerialization.
HRESULT KerbPackInteractiveLogon(
    PCWSTR pszDomain,
    PCWSTR pszUsername,
    PCWSTR pszPassword,
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    PWSTR* ppszPackage);

// Used to unpack a credential for SetSerialization.
HRESULT KerbUnpackInteractiveLogon(
    const CREDENTIAL_PROVIDER_SERIALIZATION* pcpcs,
    PWSTR* ppszDomain,
    PWSTR* ppszUsername,
    PWSTR* ppszPassword);

HRESULT CoAllocString(PCWSTR pszIn, PWSTR* ppszOut);
HRESULT CoAllocStringN(PCWSTR pszIn, size_t len, PWSTR* ppszOut);

// Smart pointer for CoTaskMemFree
template <typename T>
class CoTaskMemPtr {
public:
    CoTaskMemPtr() : _ptr(nullptr) {}
    ~CoTaskMemPtr() {
        if (_ptr) {
            CoTaskMemFree(_ptr);
            _ptr = nullptr;
        }
    }
    T** operator&() { return &_ptr; }
    operator T*() const { return _ptr; }
    T* _ptr;
};
