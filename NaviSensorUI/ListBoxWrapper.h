#pragma once

#include "GenericControlWrapper.h"

class CListBoxWrapper : public CGenericControlWrapper
{
    public:
        CListBoxWrapper (UINT uiControlID) : CGenericControlWrapper (uiControlID) {}
        CListBoxWrapper (HWND hwndParent, UINT uiControlID);
        
        int AddString (const char *pszString, LPARAM lParam = 0);
        void DeleteString (const int nIndex);
        int GetCurSel (void);
        int GetCount (void);
        int GetItemTextLength (const int nIndex);
        int GetItemText (const int nIndex, char *pchBuffer);
        void SetCurSel (const int nSelection);
        void SetItemData (const int nItem, const LPARAM lData);
        LPARAM GetItemData (const int nIndex);
        LPARAM GetSelectedData ();
};
