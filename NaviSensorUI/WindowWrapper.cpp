#include <StdLib.h>
#include <Windows.h>
#include "WindowWrapper.h"

CWindowWrapper *CWindowWrapper::m_pInstances [MAX_INSTANCES];
int             CWindowWrapper::m_nInstCount = 0;

CWindowWrapper::CWindowWrapper (HINSTANCE hInstance, HWND hwndParent, const char *pszClassName, HMENU hmnuMenu, const char *pszIcon, const char *pszCursor, const UINT uiBrush)
{    
    AddInstance (this);
    
    if (pszClassName != NULL)
        strncpy (m_chClassName, pszClassName, sizeof (m_chClassName));

    m_bIsDialogBox = FALSE;
    m_dwLastError  = 0;    
    m_hwndParent   = hwndParent;
    m_hInstance    = hInstance;
    m_hwndHandle   = NULL;
    m_hmnuMenu     = hmnuMenu;
    m_pszIcon      = (char *) pszIcon;
    m_pszCursor    = (char *) pszCursor;
    m_uiBrush      = uiBrush;
    m_aClass       = pszClassName ? RegisterClass () : 0;
}

CWindowWrapper::CWindowWrapper (HWND hwndParent, UINT uiControlID)
{
    memset (m_chClassName, 0, sizeof (m_chClassName));

    m_bIsDialogBox = FALSE;
    m_dwLastError  = 0;    
    m_hwndParent   = hwndParent;
    m_hwndHandle   = GetDlgItem (hwndParent, uiControlID); // May be yet NULL
    m_pszIcon      = NULL;
    m_pszCursor    = NULL;
    m_uiBrush      = NULL;
    m_hmnuMenu     = (HMENU) uiControlID;
    
    if (m_hwndHandle == NULL)
    {
        m_hInstance = (HINSTANCE) GetWindowLongPtr (hwndParent, GWLP_HINSTANCE);
        
        memset (m_chClassName, 0, sizeof (m_chClassName));
    }
    else
    {
        m_hInstance = (HINSTANCE) GetWindowLongPtr (m_hwndHandle, GWLP_HINSTANCE);
    
        GetClassName (m_hwndHandle, m_chClassName, sizeof (m_chClassName));
    }
    
    if (m_hwndHandle)
        AddInstance (this);
}

CWindowWrapper::~CWindowWrapper ()
{
    if (m_hwndHandle != NULL)
    {    
        RemoveInstance (m_hwndHandle);
        
        Destroy ();
    }
}

BOOL CWindowWrapper::RegisterClass ()
{
    WNDCLASS wcClass;
    
    memset (& wcClass, 0, sizeof (wcClass));
    
    wcClass.style         = CS_VREDRAW | CS_HREDRAW;
    wcClass.lpfnWndProc   = CWindowWrapper::WindowProc;
    wcClass.hInstance     = m_hInstance;
    wcClass.hIcon         = LoadIcon (NULL, m_pszIcon);
    wcClass.hCursor       = LoadCursor (NULL, m_pszCursor);
    wcClass.hbrBackground = (HBRUSH) GetStockObject (m_uiBrush);
    wcClass.lpszClassName = m_chClassName;
    
    m_aClass = ::RegisterClass (& wcClass);
    
    return m_aClass != 0;
}
                    
LRESULT CALLBACK CWindowWrapper::WindowProc (HWND hwndHandle, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    CWindowWrapper *pInstance;

    if (uiMessage == WM_CREATE)
    {
        CREATESTRUCT   *pData = (CREATESTRUCT *) lParam;
        CWindowWrapper *pThis = (CWindowWrapper *) pData->lpCreateParams;
        
        pThis->m_hwndHandle = hwndHandle;
    }
    else if (uiMessage == WM_INITDIALOG)
    {
        CWindowWrapper *pThis = (CWindowWrapper *) lParam;

        pThis->m_hwndHandle = hwndHandle;
    }

    pInstance = FindInstance (hwndHandle);

    if (pInstance == NULL)
        pInstance = (CWindowWrapper *) GetWindowLong (hwndHandle, GWL_USERDATA);

    if (pInstance == NULL)
    {
        if (GetWindowLong (hwndHandle, DWL_DLGPROC) != NULL)
            return uiMessage == WM_INITDIALOG;
        else
            return DefWindowProc (hwndHandle, uiMessage, wParam, lParam);
    }

    if (pInstance == NULL && m_nInstCount > 0)
    {
        pInstance = m_pInstances [m_nInstCount-1];
        
        pInstance->m_hwndHandle = hwndHandle;
    }
    
    return (pInstance == NULL) ? 0 : pInstance->OnMessage (uiMessage, wParam, lParam);
}

BOOL CWindowWrapper::Create (const char *pszText, const int nX, const int nY, const int nWidth, const int nHeight, const UINT uiStyle)
{
    m_hwndHandle = CreateWindow (m_chClassName, pszText, uiStyle, nX, nY, nWidth, nHeight, m_hwndParent, m_hmnuMenu, m_hInstance, this);
    
    if (m_hwndHandle == NULL)
        m_dwLastError = GetLastError ();
        
    return m_hwndHandle != NULL;
}

BOOL CWindowWrapper::Destroy ()
{
    return (m_hwndHandle != NULL) ? DestroyWindow (m_hwndHandle) : NULL;
}

LRESULT CWindowWrapper::OnMessage (UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;
    
    switch (uiMessage)
    {
        case WM_CREATE:
            lResult = OnCreate ((CREATESTRUCT *) lParam); break;
            
        case WM_NOTIFY:
            lResult = OnNotify ((NMHDR *) lParam); break;
            
        case WM_SIZE:
            lResult = OnSize (wParam, LOWORD (lParam), HIWORD (lParam)); break;
        
        case WM_COMMAND:
            lResult = OnCommand (wParam, lParam); break;
            
        case WM_SYSCOMMAND:
            lResult = OnSysCommand (wParam, lParam); break;
            
        case WM_TIMER:
            lResult = OnTimer(wParam); break;

        case WM_DESTROY:
            OnDestroy ();

            RemoveInstance (m_hwndHandle);

        default:
            lResult = m_bIsDialogBox ? 0 : DefWindowProc (m_hwndHandle, uiMessage, wParam, lParam);
    }
    
    return lResult;
}

LRESULT CWindowWrapper::OnNotify (NMHDR *pHeader)
{
    return FALSE;
}

LRESULT CWindowWrapper::OnSize (const DWORD dwRequestType, const WORD wX, const WORD wY)
{
    return TRUE;
}

LRESULT CWindowWrapper::OnDestroy ()
{
    return FALSE;
}

LRESULT CWindowWrapper::OnCommand (WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

LRESULT CWindowWrapper::OnCreate (CREATESTRUCT *pData)
{
    return FALSE;
}

LRESULT CWindowWrapper::OnTimer (UINT uiTimerID)
{
    return DefWindowProc(m_hwndHandle, WM_TIMER, uiTimerID, 0);
}

LRESULT CWindowWrapper::OnSysCommand (WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc (m_hwndHandle, WM_SYSCOMMAND, wParam, lParam);;
}

void CWindowWrapper::AddInstance (CWindowWrapper *pInstance)
{
    m_pInstances [m_nInstCount++] = pInstance;
}

void CWindowWrapper::RemoveInstance (HWND hwndHandle)
{
    int             nIndex    = 0;
    CWindowWrapper *pInstance = FindInstance (hwndHandle, & nIndex);
    
    if (pInstance)
    {
        if (m_nInstCount - nIndex > 1)
            memmove (m_pInstances + nIndex, m_pInstances + (nIndex + 1), sizeof (CWindowWrapper *) * (m_nInstCount - nIndex - 1));

        -- m_nInstCount;
    }
}

CWindowWrapper *CWindowWrapper::FindInstance (HWND hwndHandle, int *pnIndex)
{
    CWindowWrapper *pResult;
    int             i;
    
    for (i = 0, pResult = NULL; i < m_nInstCount; ++ i)
    {
        if (m_pInstances [i]->m_hwndHandle == hwndHandle)
        {
            pResult = m_pInstances [i];
            
            if (pnIndex != NULL)
                *pnIndex = i;
                
            break;
        }
    }
    
    return pResult;
}

int CWindowWrapper::MessageBox (const char *pszText, const char *pszCaption, UINT uiType)
{
    return ::MessageBox (m_hwndHandle, pszText, pszCaption, uiType);
}

void CWindowWrapper::MessageLoop ()
{
    MSG msgMessage;
    
    while (GetMessage (& msgMessage, NULL, NULL, NULL))
    {
        if (!IsDialogMessage (& msgMessage))
        {
            TranslateMessage (& msgMessage);
            DispatchMessage (& msgMessage);
        }
    }
}

LRESULT CWindowWrapper::SendMessage (UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    return ::SendMessage (m_hwndHandle, uiMessage, wParam, lParam);
}

void CWindowWrapper::Enable (const BOOL bEnabled)
{
    ::EnableWindow (m_hwndHandle, bEnabled);
}

int CWindowWrapper::GetText (char *pchBuffer, const int nSize)
{
    return GetWindowText (m_hwndHandle, pchBuffer, nSize);
}

void CWindowWrapper::SetText (const char *pszString)
{
    SetWindowText (m_hwndHandle, pszString);
}

BOOL CWindowWrapper::IsDialogMessage (MSG *pMessage)
{
    BOOL            bResult;
    CWindowWrapper *pInstance;
    int             i;

    for (i = 0, bResult = FALSE; i < m_nInstCount && !bResult ; ++ i)
    {
        pInstance = m_pInstances [i];

        if (pInstance->IsDialogBox () && ::IsDialogMessage (pInstance->GetHandle (), pMessage))
            bResult = TRUE;
    }

    return bResult;
}
