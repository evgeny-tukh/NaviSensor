#pragma once

#include "DialogWrapper.h"

class CInputBox : public CDialogWrapper
{
    public:
        CInputBox (HINSTANCE instance, HWND parent, const char *title, const char *prompt, char *buffer, const int size);

    protected:
        char *buffer,
             *prompt,
             *title;
        int   size;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual void OnOK ();
};
