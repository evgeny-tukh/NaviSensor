#pragma once

class CWindowWrapper;

#define MAX_INSTANCES   100

class CWindowWrapper
{
    public:
        CWindowWrapper (HINSTANCE   hInstance,
                        HWND        hwndParent,
                        const char *pszClassName,
                        HMENU       hmnuMenu  = NULL,
                        const char *pszIcon   = IDI_APPLICATION,
                        const char *pszCursor = IDC_ARROW,
                        const UINT  uiBrush   = WHITE_BRUSH);
        CWindowWrapper (HWND hwndParent, UINT uiControlID);
        virtual ~CWindowWrapper ();
        
        static void MessageLoop ();
        
        BOOL Create (const char *pszText = NULL, const int nX = CW_USEDEFAULT, const int nY = CW_USEDEFAULT, const int nWidth = CW_USEDEFAULT, const int nHeight = CW_USEDEFAULT,
                     const UINT uiStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW);
        BOOL Destroy ();
        
        int MessageBox (const char *pszText, const char *pszCaption, UINT uiType);
        
        LRESULT SendMessage (UINT uiMessage, WPARAM wParam = 0, LPARAM lParam = 0);
        
        void Enable (const BOOL bEnabled = TRUE);
        int GetText (char *pchBuffer, const int nSize);
        void SetText (const char *pszString);
        
        inline HWND GetHandle () { return m_hwndHandle; }
        inline BOOL GetClientRect (RECT *pRect) { return ::GetClientRect (m_hwndHandle, pRect); }
        inline BOOL GetWindowRect (RECT *pRect) { return ::GetWindowRect (m_hwndHandle, pRect); }
        inline void Show (const int nShowCommand) { ShowWindow (m_hwndHandle, nShowCommand); }
        inline void Update () { UpdateWindow (m_hwndHandle); }
        inline void Move (const int nX, const int nY, const int nWidth, const int nHeight, const BOOL bRepaint)
        {
            ::MoveWindow (m_hwndHandle, nX, nY, nWidth, nHeight, bRepaint);
        }

        inline UINT IsDlgButtonChecked (const int nIDButton)
        {
            return ::IsDlgButtonChecked (m_hwndHandle, nIDButton);
        }

        inline BOOL CheckDlgButton (const int nIDButton, const UINT uiCheck)
        {
            return ::CheckDlgButton (m_hwndHandle, nIDButton, uiCheck);
        }
        
    protected:
        ATOM      m_aClass;
        HWND      m_hwndHandle,
                  m_hwndParent;
        HMENU     m_hmnuMenu;
        HINSTANCE m_hInstance;
        char     *m_pszIcon,
                 *m_pszCursor,
                  m_chClassName [100];
        UINT      m_uiBrush;
        DWORD     m_dwLastError;
        
        virtual BOOL RegisterClass ();
                            
        static LRESULT CALLBACK WindowProc (HWND hwndHandle, UINT uiMessage, WPARAM wParam, LPARAM lParam);

        virtual LRESULT OnCreate (CREATESTRUCT *pData);
        virtual LRESULT OnMessage (UINT uiMessage, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysCommand (WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSize (const DWORD dwRequestType, const WORD wWidth, const WORD wHeight);
        virtual LRESULT OnNotify (NMHDR *pHeader);
        
    private:
        static CWindowWrapper *m_pInstances [MAX_INSTANCES];
        static int             m_nInstCount;
        
        static void AddInstance (CWindowWrapper *pInstance);
        static void RemoveInstance (HWND hwndHandle);
        static CWindowWrapper *FindInstance (HWND hwndHandle, int *pnIndex = NULL);
};