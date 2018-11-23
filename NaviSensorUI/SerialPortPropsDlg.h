#pragma once

#include "SensorConfig.h"
#include "DialogWrapper.h"
#include "EditWrapper.h"
#include "ComboBoxWrapper.h"

class SerialPortPropsDlg : public CDialogWrapper
{
    public:
        SerialPortPropsDlg (HINSTANCE instance, HWND parent, Sensors::SerialParams *params);

    protected:
        Sensors::SerialParams *params;
        CComboBoxWrapper       port, baud, parity, byteSize, stopBits;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual void OnOK ();
};
