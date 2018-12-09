#pragma once

#include "GenericControlWrapper.h"

class CTreeCtrlWrapper : public CGenericControlWrapper
{
    public:
        CTreeCtrlWrapper (UINT uiControlID) : CGenericControlWrapper (uiControlID) {}
        CTreeCtrlWrapper (HWND hwndParent, UINT uiControlID);
        
        void DeleteAllItems (void);
        void DeleteItem (HTREEITEM htiItem);
        HTREEITEM InsertItem (const char *pszText, LPARAM lParam = 0, HTREEITEM htiParent = TVI_ROOT, HTREEITEM htiInsertAfter = TVI_LAST);
        void SetItemCheck (HTREEITEM htiItem, const BOOL bChecked);
        BOOL GetItemCheck (HTREEITEM htiItem);
        LPARAM GetItemData (HTREEITEM htiItem);
        HTREEITEM GetSelectedItem (void);
        HTREEITEM GetNextItem (HTREEITEM htiItem, WPARAM wWhich);
        void SetSelectedItem (HTREEITEM htiItem);
        void EnableCheckBoxes (const BOOL bEnable = TRUE);
        unsigned int GetIndent ();
};
