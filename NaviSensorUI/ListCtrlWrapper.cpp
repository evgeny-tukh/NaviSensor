#include "ListCtrlWrapper.h"

CListCtrlWrapper::CListCtrlWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_LISTVIEW);
}

void CListCtrlWrapper::AddColumn (const char *pszTitle, const int nWidth)
{
    LVCOLUMN lvcData;
    
    memset (& lvcData, 0, sizeof (lvcData));
    
    lvcData.mask    = LVCF_WIDTH | LVCF_TEXT;
    lvcData.cx      = nWidth;
    lvcData.pszText = (char *) pszTitle;
    
    SendMessage (LVM_INSERTCOLUMN, 0xFFFF, (LPARAM) & lvcData);
}

void CListCtrlWrapper::AddColumnW (const WCHAR *pszTitle, const int nWidth)
{
    LVCOLUMNW lvcData;

    memset (& lvcData, 0, sizeof (lvcData));

    lvcData.mask    = LVCF_WIDTH | LVCF_TEXT;
    lvcData.cx      = nWidth;
    lvcData.pszText = (WCHAR *) pszTitle;

    SendMessage (LVM_INSERTCOLUMNW, 0xFFFF, (LPARAM) & lvcData);
}

void CListCtrlWrapper::DeleteAllItems ()
{
    SendMessage (LVM_DELETEALLITEMS);
}

int CListCtrlWrapper::GetItemCount ()
{
    return SendMessage (LVM_GETITEMCOUNT);
}

int CListCtrlWrapper::AddItem (const char *pszText, LPARAM lParam)
{
    LVITEM lviItem;
    
    memset (& lviItem, 0, sizeof (lviItem));
    
    lviItem.mask    = LVIF_TEXT | LVIF_PARAM;
    lviItem.iItem   = 0xFFFF;
    lviItem.pszText = (char *) pszText;
    lviItem.lParam  = lParam;
    
    return (int) SendMessage (LVM_INSERTITEM, 0, (LPARAM) & lviItem);
}

int CListCtrlWrapper::AddItemW (const WCHAR *pszText, LPARAM lParam)
{
    LVITEMW lviItem;

    memset (& lviItem, 0, sizeof (lviItem));

    lviItem.mask    = LVIF_TEXT | LVIF_PARAM;
    lviItem.iItem   = 0xFFFF;
    lviItem.pszText = (WCHAR *)pszText;
    lviItem.lParam  = lParam;

    return (int) SendMessage (LVM_INSERTITEMW, 0, (LPARAM) & lviItem);
}

void CListCtrlWrapper::DeleteItem (const int nItem)
{
    SendMessage (LVM_DELETEITEM, nItem);
}

void CListCtrlWrapper::SetItemState (const int nItem, const int nStateMask, const int nState)
{
    LVITEM lviItem;
    
    memset (& lviItem, 0, sizeof (lviItem));
    
    lviItem.mask      = LVIF_STATE;
    lviItem.iItem     = nItem;
    lviItem.stateMask = nStateMask;
    lviItem.state     = nState;
    
    SendMessage (LVM_SETITEMSTATE, nItem, (LPARAM) & lviItem);
}

void CListCtrlWrapper::SetItemText (const int nItem, const int nColumn, const char *pszText)
{
    LVITEM lviItem;
    
    memset (& lviItem, 0, sizeof (lviItem));
    
    lviItem.mask     = LVIF_TEXT;
    lviItem.iItem    = nItem;
    lviItem.pszText  = (char *) pszText;
    lviItem.iSubItem = nColumn;
    
    SendMessage (LVM_SETITEMTEXT, nItem, (LPARAM) & lviItem);
}

void CListCtrlWrapper::SetItemTextW (const int nItem, const int nColumn, const WCHAR *pszText)
{
    LVITEMW lviItem;

    memset (& lviItem, 0, sizeof (lviItem));

    lviItem.mask     = LVIF_TEXT;
    lviItem.iItem    = nItem;
    lviItem.pszText  = (WCHAR *)pszText;
    lviItem.iSubItem = nColumn;

    SendMessage (LVM_SETITEMTEXTW, nItem, (LPARAM) & lviItem);
}

void CListCtrlWrapper::SetWholeLineSelection ()
{
    SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
}

int CListCtrlWrapper::GetSelectedItem ()
{
    return (int) SendMessage (LVM_GETNEXTITEM, -1, LVNI_SELECTED);
}

LPARAM CListCtrlWrapper::GetItemData (const int nItem)
{
    LVITEM lviItem;
    
    memset (& lviItem, 0, sizeof (lviItem));
    
    lviItem.mask  = LVIF_PARAM;
    lviItem.iItem = nItem;
    
    if (SendMessage (LVM_GETITEM, nItem, (LPARAM) & lviItem))
        return lviItem.lParam;
    else
        return 0;
}

int CListCtrlWrapper::GetItemText (const int nItem, const int nColumn, char *pchBuffer, const int nSize)
{
    LVITEM lviItem;
    
    memset (& lviItem, 0, sizeof (lviItem));
    memset (pchBuffer, 0, nSize);
    
    lviItem.mask       = LVIF_TEXT;
    lviItem.iItem      = nItem;
    lviItem.iSubItem   = nColumn;
    lviItem.pszText    = pchBuffer;
    lviItem.cchTextMax = nSize;
    
    return SendMessage (LVM_GETITEMTEXT, nItem, (LPARAM) & lviItem);
}

void CListCtrlWrapper::SetItemData (const int nItem, const LPARAM lData)
{
    LVITEM lviItem;
    
    memset (& lviItem, 0, sizeof (lviItem));
    
    lviItem.mask   = LVIF_PARAM;
    lviItem.iItem  = nItem;
    lviItem.lParam = lData;
    
    SendMessage (LVM_SETITEM, nItem, (LPARAM) & lviItem);
}
