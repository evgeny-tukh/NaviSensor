#include <StdLib.h>
#include <stdio.h>
#include <Windows.h>
#include "DialogWrapper.h"

CDialogWrapper::CDialogWrapper (HINSTANCE hInstance, HWND hwndParent, const UINT uiResourceID) : CWindowWrapper (hInstance, hwndParent, NULL, NULL, NULL, NULL, NULL)
{
    m_bIsDialogBox = TRUE;
    m_hwndHandle   = NULL;
    m_uiResourceID = uiResourceID;
}

CDialogWrapper::~CDialogWrapper ()
{
    if (IsWindow (m_hwndHandle))
        ::EndDialog (m_hwndHandle, 0);
        
    m_hwndHandle = NULL;
}

INT_PTR CALLBACK ModelessDialogProc (HWND hwndHandle, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    return uiMessage == WM_INITDIALOG;
}

BOOL CALLBACK CDialogWrapper::DialogProc (HWND hwndHandle, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    INT_PTR lResult = CWindowWrapper::WindowProc (hwndHandle, uiMessage, wParam, lParam);

    return uiMessage == WM_INITDIALOG;
}


BOOL CDialogWrapper::EndDialog (const int nCode)
{
    return ::EndDialog (m_hwndHandle, nCode);
}

void CDialogWrapper::Show ()
{
    m_hwndHandle = CreateDialogParam (m_hInstance, MAKEINTRESOURCE (m_uiResourceID), m_hwndParent, DialogProc, (LPARAM) this);

    ShowWindow (m_hwndHandle, SW_SHOW);
    //UpdateWindow (m_hwndHandle);
}

BOOL CDialogWrapper::Execute ()
{
    return DialogBoxParam (m_hInstance, MAKEINTRESOURCE (m_uiResourceID), m_hwndParent, DialogProc, (LPARAM) this);
}

LRESULT CDialogWrapper::OnMessage (UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    
    switch (uiMessage)
    {
        case WM_TIMER:
            lResult = OnTimer (wParam); break;

        case WM_INITDIALOG:
            lResult = OnInitDialog (wParam, lParam); break;            

        case WM_NOTIFY:
            lResult = FALSE; OnNotify ((NMHDR *) lParam); break;
            
        case WM_COMMAND:
            lResult = FALSE; OnCommand (wParam, lParam); break;
            
        default:
            lResult = FALSE;
    }
    
    return lResult;
}

BOOL CDialogWrapper::OnInitDialog (WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

LRESULT CDialogWrapper::OnCommand (WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD (wParam))
    {
        case IDOK:
            OnOK (); break;
            
        case IDCANCEL:
            OnCancel (); break;
            
        default:
            CWindowWrapper::OnCommand (wParam, lParam);
    }
    
    return FALSE;
}

void CDialogWrapper::OnOK ()
{
    EndDialog (IDOK);
}

void CDialogWrapper::OnCancel ()
{
    EndDialog (IDCANCEL);
}

void CDialogWrapper::SetItemText (const UINT uiControlID, const char *pszText)
{
    ::SetDlgItemText (m_hwndHandle, uiControlID, pszText);
}

char *CDialogWrapper::GetItemText (const UINT uiControlID, char *pchText, const int nSize)
{
    if (pchText && nSize > 0)
        GetDlgItemText (m_hwndHandle, uiControlID, pchText, nSize);

    return pchText;
}