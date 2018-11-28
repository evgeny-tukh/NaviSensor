#pragma once

#include "GenericControlWrapper.h"

class CListCtrlWrapper : public CGenericControlWrapper
{
    public:
        CListCtrlWrapper (UINT uiControlID) : CGenericControlWrapper (uiControlID) {}
        CListCtrlWrapper (HWND hwndParent, UINT uiControlID);
        
        void AddColumn (const char *pszTitle, const int nWidth);
        void AddColumnW (const WCHAR *pszTitle, const int nWidth);
        void DeleteAllItems (void);
        int AddItem (const char *pszText, LPARAM lParam = 0, const int nImage = -1);
        int AddItemW (const WCHAR *pszText, LPARAM lParam = 0, const int nImage = -1);
        void DeleteItem (const int nItem);
        void SetItemText (const int nItem, const int nColumn, const char *pszText);
        void SetItemTextW (const int nItem, const int nColumn, const WCHAR *pszText);
        void SetItemState (const int nItem, const int nStateMask, const int nState);
        void SetItemImage (const int nItem, const int nImage);
        void SetItemData (const int nItem, const LPARAM lData);
        LPARAM GetItemData (const int nItem);
        void SetWholeLineSelection ();
        int GetSelectedItem (void);
        int GetItemCount (void);
        int GetItemText (const int nItem, const int nColumn, char *pchBuffer, const int nSize);
        
        inline void SetExtendedStyle (const UINT uiMask, const UINT uiStyle) { SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, uiMask, uiStyle); }
};
