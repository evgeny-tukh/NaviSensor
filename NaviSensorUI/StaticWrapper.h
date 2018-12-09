#pragma once

#include "GenericControlWrapper.h"

class CStaticWrapper : public CGenericControlWrapper
{
    public:
        CStaticWrapper (UINT uiControlID) : CGenericControlWrapper(uiControlID) {}
        CStaticWrapper (HWND hwndParent, UINT uiControlID);
};
