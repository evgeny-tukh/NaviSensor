#pragma once

#include "SensorConfig.h"
#include "DialogWrapper.h"
#include "EditWrapper.h"
#include "ComboBoxWrapper.h"

class SensorPropsDlg : public CDialogWrapper
{
    public:
        SensorPropsDlg (HINSTANCE instance, HWND parent, Sensors::SensorConfig *sensorCfg);

    protected:
        Sensors::SensorConfig *sensorCfg;
        CComboBoxWrapper       type, connection;
        CEditWrapper           name, paramString;

        virtual LRESULT OnCommand (WPARAM wParam, LPARAM lParam);
        virtual BOOL OnInitDialog (WPARAM wParam, LPARAM lParam);
        virtual void OnOK ();

        void showParameterString ();
};
