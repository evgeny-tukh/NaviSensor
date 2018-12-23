#include "ButtonWrapper.h"

CButtonWrapper::CButtonWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_BUTTON);
}

BOOL CButtonWrapper::IsChecked ()
{
    return /*IsDlgButtonChecked (m_uiControlID)*/SendMessage (BM_GETCHECK ) == BST_CHECKED;
}

void CButtonWrapper::Check (const BOOL bChecked)
{
    //CheckDlgButton (m_uiControlID, bChecked ? BST_CHECKED : BST_UNCHECKED);
    SendMessage (BM_SETCHECK, bChecked ? BST_CHECKED : BST_UNCHECKED, 0);
}

