#pragma once

#include <StdLib.h>
#include <Windows.h>
#include <Commctrl.h>
#include "WindowWrapper.h"

class CGenericControlWrapper : public CWindowWrapper
{
    public:
        CGenericControlWrapper (UINT uiControlID);
        CGenericControlWrapper (HWND hwndParent, UINT uiControlID);

        void Attach (HWND hwndControl);
        void FindAndAttach (HWND hwndParent);
        
        BOOL CreateControl (const int nX, const int nY, const int nWidth, const int nHeight, const UINT uiControlStyle = 0, const char *pszText = "");
        
    protected:
        UINT m_uiControlID;
};