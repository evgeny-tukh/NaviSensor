#pragma once

#include "GenericControlWrapper.h"

class CButtonWrapper : public CGenericControlWrapper
{
    public:
        CButtonWrapper (UINT uiControlID) : CGenericControlWrapper (uiControlID) {}
        CButtonWrapper (HWND hwndParent, UINT uiControlID);
        
        BOOL IsChecked (void);
        void Check (const BOOL bChecked = TRUE);
};
