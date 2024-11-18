/*---------------------------------------------------------*\
| RGBController_SkyloongKeyboard.h                          |
|                                                           |
|   RGBController for Skyloong Keyboard                    |
|                                                           |
|   Givo (givowo)                               30 Jun 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include "RGBController.h"
#include "SkyloongKeyboardController.h"

#define BRIGHTNESS_MIN      0
#define BRIGHTNESS_MAX      134

class RGBController_SkyloongKeyboard : public RGBController
{
public:
    RGBController_SkyloongKeyboard(SkyloongKeyboardController* controller_ptr);
    ~RGBController_SkyloongKeyboard();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

    void        KeepaliveThreadFunction();

private:
    SkyloongKeyboardController*   controller;

    std::atomic<bool>   keepalive_thread_run;
    std::thread*        keepalive_thread;
};
