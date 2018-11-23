#pragma once

#include "GenericControlWrapper.h"

class CEditWrapper : public CGenericControlWrapper
{
    public:
        CEditWrapper (UINT uiControlID) : CGenericControlWrapper (uiControlID) {}
        CEditWrapper (HWND hwndParent, UINT uiControlID);
        
        void SetTextLimit (const int nLimit);
        void SetInt (const int nValue, const BOOL bSigned = FALSE);
        int GetInt (const BOOL bSigned = FALSE);
};

class CIPAddrControlWrapper : public CEditWrapper
{
    public:
        CIPAddrControlWrapper (UINT uiControlID) : CEditWrapper (uiControlID) {}
        CIPAddrControlWrapper (HWND hwndParent, UINT uiControlID);

        void SetAddr (in_addr address);
        in_addr GetAddr ();
};