#include "TreeCtrlWrapper.h"

CTreeCtrlWrapper::CTreeCtrlWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_TREEVIEW);
}

void CTreeCtrlWrapper::DeleteAllItems ()
{
    SendMessage (TVM_DELETEITEM, 0, NULL);
}

void CTreeCtrlWrapper::DeleteItem (HTREEITEM htiItem)
{
    SendMessage (TVM_DELETEITEM, 0, (LPARAM) htiItem);
}

HTREEITEM CTreeCtrlWrapper::InsertItem (const char *pszText, LPARAM lParam, HTREEITEM htiParent, HTREEITEM htiInsertAfter)
{
    TVINSERTSTRUCT tviItem;
    
    memset (& tviItem, 0, sizeof (tviItem));

    tviItem.hInsertAfter = htiInsertAfter;
    tviItem.hParent      = htiParent;
    tviItem.item.mask    = TVIF_TEXT | TVIF_PARAM;
    tviItem.item.pszText = (char *) pszText;
    tviItem.item.lParam  = lParam;

    return (HTREEITEM) SendMessage (TVM_INSERTITEM, 0, (LPARAM) & tviItem);
}

void CTreeCtrlWrapper::SetItemImage (HTREEITEM htiItem, const int nImage)
{
    TVITEM tviItem;

    memset (& tviItem, 0, sizeof (tviItem));

    tviItem.mask           = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tviItem.hItem          = htiItem;
    tviItem.iImage         =
    tviItem.iSelectedImage = nImage;

    SendMessage (TVM_SETITEM, 0, (LPARAM) & tviItem);
}

HTREEITEM CTreeCtrlWrapper::InsertItem (const char *pszText, const int nImage, LPARAM lParam, HTREEITEM htiParent, HTREEITEM htiInsertAfter)
{
    TVINSERTSTRUCT tviItem;

    memset (&tviItem, 0, sizeof (tviItem));

    tviItem.hInsertAfter        = htiInsertAfter;
    tviItem.hParent             = htiParent;
    tviItem.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tviItem.item.pszText        = (char *) pszText;
    tviItem.item.lParam         = lParam;
    tviItem.item.iImage         =
    tviItem.item.iSelectedImage = nImage;

    return (HTREEITEM) SendMessage (TVM_INSERTITEM, 0, (LPARAM) & tviItem);
}

void CTreeCtrlWrapper::SetItemCheck (HTREEITEM htiItem, const BOOL bChecked)
{
    TVITEM  tviItem;
    int     nState = (int) bChecked + 1;
    LRESULT lResult;
    
    memset (& tviItem, 0, sizeof (tviItem));
    
    tviItem.mask      = TVIF_STATE;
    tviItem.state     = INDEXTOSTATEIMAGEMASK (nState);
    tviItem.stateMask = TVIS_STATEIMAGEMASK;
    tviItem.hItem     = htiItem;
    
    lResult = SendMessage (TVM_SETITEM, 0, (LPARAM) & tviItem);
    //TreeView_SetCheckState (GetWindowHandle (), htiItem, bChecked);
}

BOOL CTreeCtrlWrapper::GetItemCheck (HTREEITEM htiItem)
{
    LRESULT lResult = SendMessage (TVM_GETITEMSTATE, (WPARAM) htiItem, (LPARAM) TVIS_STATEIMAGEMASK);
    
    return (lResult >> 12) -1;
    //return TreeView_GetCheckState (GetWindowHandle (), htiItem) > 0;
}

LPARAM CTreeCtrlWrapper::GetItemData (HTREEITEM htiItem)
{
    TVITEM tviItem;
    
    memset (& tviItem, 0, sizeof (tviItem));
    
    tviItem.mask  = TVIF_PARAM;
    tviItem.hItem = htiItem;
    
    if (SendMessage (TVM_GETITEM, 0, (LPARAM) & tviItem))
        return tviItem.lParam;
    else
        return 0;
}

unsigned int CTreeCtrlWrapper::GetIndent()
{
    return SendMessage (TVM_GETINDENT);
}

HTREEITEM CTreeCtrlWrapper::GetNextItem (HTREEITEM htiItem, WPARAM wWhich)
{
    return (HTREEITEM) SendMessage (TVM_GETNEXTITEM, wWhich, (LPARAM) htiItem);
}

HTREEITEM CTreeCtrlWrapper::GetSelectedItem ()
{
    return (HTREEITEM) SendMessage (TVM_GETNEXTITEM, TVGN_CARET);
}

void CTreeCtrlWrapper::SetSelectedItem (HTREEITEM htiItem)
{
    SendMessage (TVM_SELECTITEM, TVGN_CARET, (LPARAM) htiItem);
}

void CTreeCtrlWrapper::EnableCheckBoxes (const BOOL bEnable)
{
    DWORD dwStyle = GetWindowLong (m_hwndHandle, GWL_STYLE);
    
    if (bEnable)
        dwStyle |= TVS_CHECKBOXES;
    else
        dwStyle &= ~TVS_CHECKBOXES;

    SetWindowLongPtr (m_hwndHandle, GWL_STYLE, dwStyle);
}
