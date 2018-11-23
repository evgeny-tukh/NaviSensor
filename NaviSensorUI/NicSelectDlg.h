#pragma once

#include <Windows.h>
#include "DialogWrapper.h"
#include "ListBoxWrapper.h"

class NicSelectDlg : public CDialogWrapper
{
    public:
        NicSelectDlg (HINSTANCE instance, HWND parent, in_addr *addr);

    protected:
        in_addr        *addr;
        CListBoxWrapper nicList;

        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual void OnOK ();
};
