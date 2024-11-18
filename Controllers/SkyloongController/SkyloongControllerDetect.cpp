/*---------------------------------------------------------*\
| SkyloongControllerDetect.cpp                              |
|                                                           |
|   Detector for Skyloong Keyboards                         |
|                                                           |
|   Givo (givowo)                               30 Jun 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "Detector.h"
#include "SkyloongKeyboardController.h"
#include "RGBController_SkyloongKeyboard.h"

/*-----------------------------------------------------*\
| Keyboard product IDs                                  |
\*-----------------------------------------------------*/
#define SKYLOONG_KEYBOARD_VID           0x1EA7
#define SKYLOONG_KEYBOARD_PID           0x0907

/******************************************************************************************\
*                                                                                          *
*   DetectSkyloongKeyboard                                                                 *
*                                                                                          *
*       Tests the USB address to see if a Skyloong Keyboard controller exists there.      *
*                                                                                          *
\******************************************************************************************/
void DetectSkyloongKeyboard(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);
    if(dev)
    {
        SkyloongKeyboardController* controller = new SkyloongKeyboardController(dev, info->path);
        gk_model* model = controller->GetModel();
        if(model)
        {
            RGBController_SkyloongKeyboard* rgb_controller  = new RGBController_SkyloongKeyboard(controller);
            rgb_controller->name = model->name;
            ResourceManager::get()->RegisterRGBController(rgb_controller);
        } else
        {
            delete controller;
        }
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------*\
| Keyboards                                                                                                                                     |
\*---------------------------------------------------------------------------------------------------------------------------------------------*/
REGISTER_HID_DETECTOR_PU("Skyloong Keyboard", DetectSkyloongKeyboard, SKYLOONG_KEYBOARD_VID, SKYLOONG_KEYBOARD_PID, 0xFF00, 0x0050);
