/*---------------------------------------------------------*\
| SkyloongKeyboardController.h                              |
|                                                           |
|   Driver for Skyloong Keyboard                           |
|                                                           |
|   Givo (givowo)                               30 Jun 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"
#include "KeyboardLayoutManager.h"

typedef struct
{
    const uint32_t          id;
    const char*             name;
    key_set                 keys;
} gk_model;

class SkyloongKeyboardController
{
public:
    SkyloongKeyboardController(hid_device* dev_handle, const char* path);
    ~SkyloongKeyboardController();

    std::string     GetDeviceLocation();
    gk_model*       GetModel();

    void            Ping();
    void            SendCommand(char command, char sub_command);
    void            SendColorPacket(std::vector<RGBColor> colors, std::vector<led> *leds, int brightness);

private:
    hid_device*     dev;
    std::string     location;
    gk_model*       model;

    uint16_t        Crc16CcittFalse(const uint8_t *buffer, uint16_t size);
    void            SetLEDefine(int address, unsigned char *le_data, int le_data_length);
    void            SaveLEDefine();
};
