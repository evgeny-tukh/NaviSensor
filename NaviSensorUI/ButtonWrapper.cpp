#include "ButtonWrapper.h"

CButtonWrapper::CButtonWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_BUTTON);
}

BOOL CButtonWrapper::IsChecked ()
{
    return IsDlgButtonChecked (m_hwndParent, m_uiControlID) == BST_CHECKED;
}

void CButtonWrapper::Check (const BOOL bChecked)
{
    CheckDlgButton (m_hwndParent, m_uiControlID, bChecked ? BST_CHECKED : BST_UNCHECKED);
}

