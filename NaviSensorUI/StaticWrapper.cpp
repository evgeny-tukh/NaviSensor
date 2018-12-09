#include "StaticWrapper.h"

CStaticWrapper::CStaticWrapper (HWND hwndParent, UINT uiControlID) : CGenericControlWrapper (hwndParent, uiControlID)
{
    strcpy (m_chClassName, WC_STATIC);
}
