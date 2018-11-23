#pragma once

#include "SensorConfig.h"
#include "DialogWrapper.h"
#include "EditWrapper.h"
#include "UpDownWrapper.h"
#include "ComboBoxWrapper.h"

class UdpConnectionPropsDlg : public CDialogWrapper
{
    public:
        UdpConnectionPropsDlg (HINSTANCE instance, HWND parent, Sensors::UdpParams *params);

    protected:
        Sensors::UdpParams    *params;
        CEditWrapper           inPort, outPort;
        CUpDownWrapper         inPortSpin, outPortSpin;
        CIPAddrControlWrapper  bind, dest;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual void OnOK ();
};
