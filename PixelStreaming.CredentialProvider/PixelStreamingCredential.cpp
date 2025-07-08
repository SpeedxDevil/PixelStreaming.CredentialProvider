// PixelStreamingCredential.cpp
#include "pch.h"
#include "PixelStreamingCredential.h"
#include "PixelStreamingCredentialProvider.h" // For CLSID_PixelStreamingCredentialProvider
#include "helpers.h" // For field IDs, string helpers, etc.
#include <new> // For std::nothrow

CPixelStreamingCredential::CPixelStreamingCredential() :
    _cRef(1),
    _cpus(CPUS_INVALID),
    _pcpce(nullptr),
    _pszUsername(nullptr),
    _pszPassword(nullptr)
{
    // Note: Initialization of strings to nullptr is important for cleanup
}

CPixelStreamingCredential::~CPixelStreamingCredential()
{
    if (_pcpce)
    {
        _pcpce->Release();
        _pcpce = nullptr;
    }
    CoTaskMemFree(_pszUsername);
    CoTaskMemFree(_pszPassword);
}

// IUnknown
IFACEMETHODIMP CPixelStreamingCredential::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CPixelStreamingCredential, ICredentialProviderCredential),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CPixelStreamingCredential::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) CPixelStreamingCredential::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

// ICredentialProviderCredential
IFACEMETHODIMP CPixelStreamingCredential::Advise(ICredentialProviderCredentialEvents* pcpce)
{
    if (pcpce == nullptr)
    {
        return E_INVALIDARG;
    }
    _pcpce = pcpce;
    _pcpce->AddRef();
    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredential::UnAdvise()
{
    if (_pcpce)
    {
        _pcpce->Release();
        _pcpce = nullptr;
    }
    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredential::SetSelected(BOOL* pbAutoLogon)
{
    if (pbAutoLogon == nullptr)
    {
        return E_INVALIDARG;
    }
    *pbAutoLogon = FALSE; // No auto logon for now
    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredential::SetDeselected()
{
    return S_OK; // Nothing to do on deselect for now
}

IFACEMETHODIMP CPixelStreamingCredential::GetFieldState(DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis)
{
    if (pcpfs == nullptr || pcpfis == nullptr)
    {
        return E_INVALIDARG;
    }

    // All fields are visible and interactive for now
    *pcpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
    *pcpfis = CPFIRS_ENABLED;

    switch (dwFieldID)
    {
    case USERNAME_FIELD_ID:
    case PASSWORD_FIELD_ID:
        break;
    default:
        return E_INVALIDARG; // Unknown field
    }
    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredential::GetStringValue(DWORD dwFieldID, PWSTR* ppsz)
{
    if (ppsz == nullptr)
    {
        return E_INVALIDARG;
    }
    *ppsz = nullptr;
    HRESULT hr = S_OK;

    switch (dwFieldID)
    {
    case USERNAME_FIELD_ID:
        hr = SHStrDupW(_pszUsername ? _pszUsername : L"", ppsz);
        break;
    case PASSWORD_FIELD_ID:
        // For security, password should not be easily retrievable as a string directly if stored securely.
        // However, the UI needs it for display (masked) or if the user types it.
        // This sample will return it, but a real provider might have more complex handling.
        hr = SHStrDupW(_pszPassword ? _pszPassword : L"", ppsz);
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }
    return hr;
}

IFACEMETHODIMP CPixelStreamingCredential::GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp)
{
    if (phbmp == nullptr)
    {
        return E_INVALIDARG;
    }
    *phbmp = nullptr;
    return E_NOTIMPL; // No bitmap fields for now
}

IFACEMETHODIMP CPixelStreamingCredential::GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppszLabel)
{
    if (pbChecked == nullptr || ppszLabel == nullptr)
    {
        return E_INVALIDARG;
    }
    return E_NOTIMPL; // No checkbox fields
}

IFACEMETHODIMP CPixelStreamingCredential::GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo)
{
    if (pdwAdjacentTo == nullptr)
    {
        return E_INVALIDARG;
    }
    // Example: if we had a submit button with SUBMIT_BUTTON_ID
    // if (dwFieldID == SUBMIT_BUTTON_ID) {
    //    *pdwAdjacentTo = PASSWORD_FIELD_ID; // Submit button next to password
    //    return S_OK;
    // }
    return E_NOTIMPL; // No submit button for now
}

IFACEMETHODIMP CPixelStreamingCredential::GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem)
{
     if (pcItems == nullptr || pdwSelectedItem == nullptr)
    {
        return E_INVALIDARG;
    }
    return E_NOTIMPL; // No combo box fields
}

IFACEMETHODIMP CPixelStreamingCredential::GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppszItem)
{
    if (ppszItem == nullptr)
    {
        return E_INVALIDARG;
    }
    return E_NOTIMPL; // No combo box fields
}

IFACEMETHODIMP CPixelStreamingCredential::SetStringValue(DWORD dwFieldID, PCWSTR psz)
{
    HRESULT hr = S_OK;
    PWSTR* pszCurrentVal = nullptr;

    switch (dwFieldID)
    {
    case USERNAME_FIELD_ID:
        pszCurrentVal = &_pszUsername;
        break;
    case PASSWORD_FIELD_ID:
        pszCurrentVal = &_pszPassword;
        break;
    default:
        return E_INVALIDARG;
    }

    CoTaskMemFree(*pszCurrentVal);
    *pszCurrentVal = nullptr;
    if (psz)
    {
        hr = SHStrDupW(psz, pszCurrentVal);
    }

    // Example: if fields change, and auto-submit is possible or UI needs update
    // if (SUCCEEDED(hr) && _pcpce) {
    //    _pcpce->CredentialsChanged(some_uint_ptr_context_from_provider_advise);
    // }
    return hr;
}

IFACEMETHODIMP CPixelStreamingCredential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
    return E_NOTIMPL; // No checkbox fields
}

IFACEMETHODIMP CPixelStreamingCredential::SetComboBoxSelectedItem(DWORD dwFieldID, DWORD dwSelectedItem)
{
    return E_NOTIMPL; // No combo box fields
}

IFACEMETHODIMP CPixelStreamingCredential::CommandLinkClicked(DWORD dwFieldID)
{
    return E_NOTIMPL; // No command link fields
}

IFACEMETHODIMP CPixelStreamingCredential::GetSerialization(
    CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
    PWSTR* ppszOptionalStatusText,
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    if (pcpgsr == nullptr || pcpcs == nullptr)
    {
        return E_INVALIDARG;
    }

    // Default to no status text or icon
    if (ppszOptionalStatusText) *ppszOptionalStatusText = nullptr;
    if (pcpsiOptionalStatusIcon) *pcpsiOptionalStatusIcon = CPSI_NONE;

    // For this basic provider, we'll try to package username and password.
    // This is a simplified example. A real provider would use KerbPackInteractiveLogon or similar.
    // and handle different CPUS scenarios.

    if (_cpus == CPUS_LOGON || _cpus == CPUS_UNLOCK_WORKSTATION)
    {
        // Only attempt logon/unlock if we have a username and password.
        if (_pszUsername && _pszUsername[0] != L'\0' && _pszPassword && _pszPassword[0] != L'\0')
        {
            // This is where you'd use a helper like KerbPackInteractiveLogon or a custom packing format.
            // For simplicity, let's just concatenate them with a separator. This is NOT secure.
            // A real implementation MUST use a secure serialization method.

            // Example of a very basic (and insecure) serialization:
            const wchar_t separator = L'\t'; // Tab separator
            size_t userLen = wcslen(_pszUsername);
            size_t passLen = wcslen(_pszPassword);
            size_t bufferSize = (userLen + 1 + passLen + 1) * sizeof(wchar_t);

            pcpcs->rgbSerialization = (BYTE*)CoTaskMemAlloc(bufferSize);
            if (pcpcs->rgbSerialization == nullptr) {
                return E_OUTOFMEMORY;
            }

            PWSTR pszBuffer = reinterpret_cast<PWSTR>(pcpcs->rgbSerialization);
            wcscpy_s(pszBuffer, userLen + 1, _pszUsername);
            pszBuffer[userLen] = separator;
            wcscpy_s(pszBuffer + userLen + 1, passLen + 1, _pszPassword);

            pcpcs->ulSerializationBufferSize = bufferSize;
            pcpcs->clsidCredentialProvider = CLSID_PixelStreamingCredentialProvider; // Needs to be defined
            pcpcs->cpus = _cpus;
            *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
            return S_OK;
        }
        else
        {
            // Not enough info to serialize for logon
            *pcpgsr = CPGSR_NO_CREDENTIAL_RETURNED;
            // Optionally set status text:
            // if (ppszOptionalStatusText) SHStrDup(L"Username and password required.", ppszOptionalStatusText);
            // if (pcpsiOptionalStatusIcon) *pcpsiOptionalStatusIcon = CPSI_ERROR;
            return S_OK;
        }
    }
    else if (_cpus == CPUS_CREDUI)
    {
         // For CredUI, we might just return the username if that's what's expected.
         // Or package both if the calling app expects to validate them.
         // This depends on what CredUIPromptForWindowsCredentials or CredUIPromptForCredentials expects.
        *pcpgsr = CPGSR_NO_CREDENTIAL_RETURNED; // Defaulting to no credential for CredUI for now
        return S_OK;
    }
    else if (_cpus == CPUS_CHANGE_PASSWORD)
    {
        // Change password scenario has specific serialization needs (old pass, new pass, confirm new pass)
        // This basic credential doesn't support that yet.
        *pcpgsr = CPGSR_NO_CREDENTIAL_RETURNED;
        return S_OK;
    }

    *pcpgsr = CPGSR_NO_CREDENTIAL_RETURNED;
    return S_OK;
}

IFACEMETHODIMP CPixelStreamingCredential::ReportResult(
    NTSTATUS ntsStatus,
    NTSTATUS ntsSubstatus,
    PWSTR* ppszOptionalStatusText,
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    // Called by the system to inform the credential of the result of a logon attempt.
    // Here you can update UI, log errors, etc.
    // For example, if ntsStatus is STATUS_ACCOUNT_LOCKED_OUT, you could display a message.

    if (ppszOptionalStatusText) *ppszOptionalStatusText = nullptr; // System will free this
    if (pcpsiOptionalStatusIcon) *pcpsiOptionalStatusIcon = CPSI_NONE;

    // Example:
    // if (ntsStatus == STATUS_WRONG_PASSWORD) {
    //    if (ppszOptionalStatusText) SHStrDup(L"Incorrect username or password.", ppszOptionalStatusText);
    //    if (pcpsiOptionalStatusIcon) *pcpsiOptionalStatusIcon = CPSI_ERROR;
    // }

    return S_OK; // The credential provider should always return S_OK from this method.
}


// Initialization method
HRESULT CPixelStreamingCredential::Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, PWSTR pszUsernameToPrefill)
{
    _cpus = cpus;

    if (pszUsernameToPrefill && pszUsernameToPrefill[0] != L'\0')
    {
        return SHStrDupW(pszUsernameToPrefill, &_pszUsername);
    }
    return S_OK;
}
