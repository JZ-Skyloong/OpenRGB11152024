/*---------------------------------------------------------*\
| RGBController_SkyloongKeyboard.cpp                        |
|                                                           |
|   RGBController for Skyloong Keyboard                     |
|                                                           |
|   Givo (givowo)                               30 Jun 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_SkyloongKeyboard.h"
#include "KeyboardLayoutManager.h"

using namespace std::chrono_literals;

/**------------------------------------------------------------------*\
    @name Skyloong Keyboard
    @category Keyboard
    @type USB
    @save :o:
    @direct :white_check_mark:
    @effects :o:
    @detectors SkyloongControllerDetect
    @comment
\*-------------------------------------------------------------------*/

RGBController_SkyloongKeyboard::RGBController_SkyloongKeyboard(SkyloongKeyboardController* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = "Skyloong Keyboard";
    vendor                      = "Skyloong";
    description                 = "Skyloong Keyboard";
    location                    = controller->GetDeviceLocation();
    type                        = DEVICE_TYPE_KEYBOARD;

    mode Direct;
    Direct.name                 = "Direct";
    Direct.value                = 0xFFFF;
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode           = MODE_COLORS_PER_LED;
    Direct.brightness_min       = BRIGHTNESS_MIN;
    Direct.brightness_max       = BRIGHTNESS_MAX;
    Direct.brightness           = BRIGHTNESS_MAX;

    modes.push_back(Direct);

    SetupZones();

    keepalive_thread_run = true;
    keepalive_thread = new std::thread(&RGBController_SkyloongKeyboard::KeepaliveThreadFunction, this);
}

RGBController_SkyloongKeyboard::~RGBController_SkyloongKeyboard()
{
    keepalive_thread_run = false;
    keepalive_thread->join();
    delete keepalive_thread;

    /*---------------------------------------------------------*\
    | Delete the matrix map                                     |
    \*---------------------------------------------------------*/
    for(unsigned int zone_index = 0; zone_index < zones.size(); zone_index++)
    {
        if(zones[zone_index].matrix_map != nullptr)
        {
            delete zones[zone_index].matrix_map;
        }
    }

    delete controller;
}

void RGBController_SkyloongKeyboard::SetupZones()
{
    gk_model* model = controller->GetModel();

    /*---------------------------------------------------------*\
    | Create the keyboard zone usiung Keyboard Layout Manager   |
    \*---------------------------------------------------------*/
    zone new_zone;
    new_zone.name               = ZONE_EN_KEYBOARD;
    new_zone.type               = ZONE_TYPE_MATRIX;

    matrix_map_type * new_map   = new matrix_map_type;
    new_zone.matrix_map         = new_map;
    new_zone.matrix_map->height = 6;
    new_zone.matrix_map->width  = 22;

    new_zone.matrix_map->map    = new unsigned int[new_map->height * new_map->width];
    new_zone.leds_count         = model->keys.size();
    new_zone.leds_min           = new_zone.leds_count;
    new_zone.leds_max           = new_zone.leds_count;

    /*---------------------------------------------------------*\
    | Matrix map still uses declared zone rows and columns      |
    |   as the packet structure depends on the matrix map       |
    \*---------------------------------------------------------*/
    for(size_t r = 0; r < new_map->height; r++)
    {
        size_t offset   = r * new_map->width;

        for(size_t c = 0; c < new_map->width; c++)
        {
            new_map->map[offset + c] = -1;
        }
    }

    for(size_t i = 0; i < model->keys.size(); i++)
    {
        size_t offset   = (model->keys[i].row * new_map->width) + model->keys[i].col;
        new_map->map[offset] = i;
    }

    /*---------------------------------------------------------*\
    | Create LEDs for the Matrix zone                           |
    |   Place keys in the layout to populate the matrix         |
    \*---------------------------------------------------------*/
    for(unsigned int led_idx = 0; led_idx < model->keys.size(); led_idx++)
    {
        led new_led;

        new_led.name            = model->keys[led_idx].name;
        new_led.value           = model->keys[led_idx].value;

        leds.push_back(new_led);
    }

    zones.push_back(new_zone);

    SetupColors();
}

void RGBController_SkyloongKeyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_SkyloongKeyboard::DeviceUpdateLEDs()
{
    controller->SendColorPacket(colors, &leds, modes[active_mode].brightness);
}

void RGBController_SkyloongKeyboard::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SkyloongKeyboard::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SkyloongKeyboard::DeviceUpdateMode()
{
}

void RGBController_SkyloongKeyboard::KeepaliveThreadFunction()
{
    while(keepalive_thread_run)
    {
        controller->Ping();
        std::this_thread::sleep_for(2s);
    }
}
