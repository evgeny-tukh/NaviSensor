#include "EditWrapper.h"

CEditWrapper::CEditWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_EDIT);
}

void CEditWrapper::SetTextLimit (const int nLimit)
{
    SendMessage (EM_SETLIMITTEXT, nLimit);
}

void CEditWrapper::SetInt (const int nValue, const BOOL bSigned)
{
    SetDlgItemInt (m_hwndParent, m_uiControlID, nValue, bSigned);
}

int CEditWrapper::GetInt (const BOOL bSigned)
{
    return GetDlgItemInt (m_hwndParent, m_uiControlID, NULL, bSigned);
}

CIPAddrControlWrapper::CIPAddrControlWrapper (HWND hwndParent, UINT uiControlID) : CEditWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_IPADDRESS);
}

void CIPAddrControlWrapper::SetAddr (in_addr address)
{
    if (address.S_un.S_addr == INADDR_ANY || address.S_un.S_addr == INADDR_BROADCAST)
        SendMessage (IPM_CLEARADDRESS);
    else
        SendMessage (IPM_SETADDRESS, 0,
                     MAKEIPADDRESS (address.S_un.S_un_b.s_b1, address.S_un.S_un_b.s_b2, address.S_un.S_un_b.s_b3, address.S_un.S_un_b.s_b4));
}

in_addr CIPAddrControlWrapper::GetAddr ()
{
    in_addr result;
    DWORD   data;

    SendMessage (IPM_GETADDRESS, 0, (LPARAM) & data);

    result.S_un.S_un_b.s_b1 = FIRST_IPADDRESS (data);
    result.S_un.S_un_b.s_b2 = SECOND_IPADDRESS (data);
    result.S_un.S_un_b.s_b3 = THIRD_IPADDRESS (data);
    result.S_un.S_un_b.s_b4 = FOURTH_IPADDRESS (data);

    return result;
}
