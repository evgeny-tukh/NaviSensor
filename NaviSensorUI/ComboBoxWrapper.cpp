#include "ComboBoxWrapper.h"

CComboBoxWrapper::CComboBoxWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_COMBOBOX);
}

int CComboBoxWrapper::AddString (const char *pszString, LPARAM lParam)
{
    int nItem = (int) SendMessage (CB_ADDSTRING, 0, (LPARAM) pszString);
    
    SendMessage (CB_SETITEMDATA, nItem, lParam);
    
    return nItem;
}

void CComboBoxWrapper::DeleteString (const int nIndex)
{
    SendMessage (CB_DELETESTRING, nIndex);
}

int CComboBoxWrapper::GetCurSel ()
{
    return (int) SendMessage (CB_GETCURSEL);
}

int CComboBoxWrapper::GetCount ()
{
    return (int) SendMessage (CB_GETCOUNT);
}

int CComboBoxWrapper::GetItemTextLength (const int nIndex)
{
    return (int) SendMessage (CB_GETLBTEXTLEN, nIndex, 0);
}

int CComboBoxWrapper::GetItemText (const int nIndex, char *pchBuffer)
{
    return (int) SendMessage (CB_GETLBTEXT, nIndex, (LPARAM) pchBuffer);
}

void CComboBoxWrapper::SetCurSel (const int nSelection)
{
    SendMessage (CB_SETCURSEL, nSelection);
}

LPARAM CComboBoxWrapper::GetItemData (const int nIndex)
{
    return SendMessage (CB_GETITEMDATA, nIndex);
}

LPARAM CComboBoxWrapper::GetSelectedData ()
{
    int nSelection = GetCurSel ();

    return nSelection >= 0 ? GetItemData (nSelection) : 0;
}

void CComboBoxWrapper::SetItemData (const int nItem, const LPARAM lData)
{
    SendMessage (CB_SETITEMDATA, nItem, lData);
}

