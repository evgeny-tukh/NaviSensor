#include "UpDownWrapper.h"

CUpDownWrapper::CUpDownWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, UPDOWN_CLASS);
}

void CUpDownWrapper::SetRange (DWORD dwMin, DWORD dwMax)
{
    SendMessage (UDM_SETRANGE32, dwMin, dwMax);
}
