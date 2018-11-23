#pragma once

#include "GenericControlWrapper.h"

class CUpDownWrapper : public CGenericControlWrapper
{
    public:
        CUpDownWrapper (UINT uiControlID) : CGenericControlWrapper (uiControlID) {}
        CUpDownWrapper (HWND hwndParent, UINT uiControlID);
        
        void SetRange (DWORD dwMin, DWORD dwMax);
};
