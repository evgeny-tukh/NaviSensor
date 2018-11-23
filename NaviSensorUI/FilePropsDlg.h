#pragma once

#include "SensorConfig.h"
#include "DialogWrapper.h"
#include "EditWrapper.h"
#include "UpDownWrapper.h"

class FilePropsDlg : public CDialogWrapper
{
    public:
        FilePropsDlg (HINSTANCE instance, HWND parent, Sensors::FileParams *params);

    protected:
        Sensors::FileParams *params;
        CEditWrapper         path, pause;
        CUpDownWrapper       pauseSpin;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual void OnOK ();
};
