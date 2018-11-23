#include <StdLib.h>
#include <Windows.h>
#include "GenericControlWrapper.h"

CGenericControlWrapper::CGenericControlWrapper (UINT uiControlID) : CWindowWrapper (NULL, uiControlID)
{
    m_uiControlID = uiControlID;
}

CGenericControlWrapper::CGenericControlWrapper (HWND hwndParent, UINT uiControlID) : CWindowWrapper (hwndParent, uiControlID)
{
    m_uiControlID = uiControlID == 0 ? (UINT) m_hmnuMenu : uiControlID;
}

void CGenericControlWrapper::Attach (HWND hwndControl)
{
    m_hwndHandle = hwndControl;
    m_hwndParent = GetParent (hwndControl);
}

void CGenericControlWrapper::FindAndAttach (HWND hwndParent)
{
    m_hwndParent = hwndParent;
    m_hwndHandle = ::GetDlgItem (hwndParent, m_uiControlID);
}

BOOL CGenericControlWrapper::CreateControl (const int nX, const int nY, const int nWidth, const int nHeight, const UINT uiControlStyle, const char *pszText)
{
    BOOL bResult;
    
    //strncpy (m_chClassName, pszClassName, sizeof (m_chClassName));
    
    bResult = CWindowWrapper::Create (pszText, nX, nY, nWidth, nHeight, WS_CHILD | WS_VISIBLE | uiControlStyle);
    
    if (bResult)
    {
        Show (SW_SHOW);
        Update ();
    }
    
    return bResult;
}

