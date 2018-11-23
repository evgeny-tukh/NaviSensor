#include "ListBoxWrapper.h"

CListBoxWrapper::CListBoxWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_LISTBOX);
}

int CListBoxWrapper::AddString (const char *pszString, LPARAM lParam)
{
    int nItem = (int) SendMessage (LB_ADDSTRING, 0, (LPARAM) pszString);
    
    SendMessage (LB_SETITEMDATA, nItem, lParam);
    
    return nItem;
}

void CListBoxWrapper::DeleteString (const int nIndex)
{
    SendMessage (LB_DELETESTRING, nIndex);
}

int CListBoxWrapper::GetCurSel ()
{
    return (int) SendMessage (LB_GETCURSEL);
}

int CListBoxWrapper::GetCount ()
{
    return (int) SendMessage (LB_GETCOUNT);
}

int CListBoxWrapper::GetItemTextLength (const int nIndex)
{
    return (int) SendMessage (LB_GETTEXTLEN, nIndex, 0);
}

int CListBoxWrapper::GetItemText (const int nIndex, char *pchBuffer)
{
    return (int) SendMessage (LB_GETTEXT, nIndex, (LPARAM) pchBuffer);
}

void CListBoxWrapper::SetCurSel (const int nSelection)
{
    SendMessage (LB_SETCURSEL, nSelection);
}

LPARAM CListBoxWrapper::GetItemData (const int nIndex)
{
    return SendMessage (LB_GETITEMDATA, nIndex);
}

LPARAM CListBoxWrapper::GetSelectedData ()
{
    int nSelection = GetCurSel ();

    return nSelection >= 0 ? GetItemData (nSelection) : 0;
}

void CListBoxWrapper::SetItemData (const int nItem, const LPARAM lData)
{
    SendMessage (LB_SETITEMDATA, nItem, lData);
}

