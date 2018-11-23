#pragma once

#include "WindowWrapper.h"

class CDialogWrapper;

#define MAX_INSTANCES   100

class CDialogWrapper : public CWindowWrapper
{
    public:
        CDialogWrapper (HINSTANCE hInstance, HWND hwndParent, const UINT uiResourceID);
        virtual ~CDialogWrapper ();
        
        int Execute ();
        void Show ();
        BOOL EndDialog (const int nCode);

        char *GetItemText (const UINT uiControlID, char *pchText, const int nSize);
        void SetItemText (const UINT uiControlID, const char *pszText);

    protected:
        UINT m_uiResourceID;
        BOOL m_bInitialized;
                
        static INT_PTR CALLBACK DialogProc (HWND hwndHandle, UINT uiMessage, WPARAM wParam, LPARAM lParam);

        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        
        virtual LRESULT OnMessage (UINT uiMessage, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        
        virtual void OnOK ();
        virtual void OnCancel ();
};