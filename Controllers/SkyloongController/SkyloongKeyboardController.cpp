/*---------------------------------------------------------*\
| SkyloongKeyboardController.cpp                            |
|                                                           |
|   Driver for Skyloong Keyboard                           |
|                                                           |
|   Givo (givowo)                               30 Jun 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <cstring>
#include "RGBControllerKeyNames.h"
#include "SkyloongKeyboardController.h"

using namespace std::chrono_literals;

#define PACKET_SIZE                 65

#define TOTAL_LED_BYTES             528
#define LED_BYTES_IN_CHUNK          56

#define SUBCOMMAND_NONE             0x00

#define INFO_MODEL_ID               0x08

#define MODE_OFFLINE                0x04
#define MODE_ONLINE                 0x05

#define LE_DEFINE_SET               0x01
#define LE_DEFINE_SAVE              0x02

enum command
{
    info        = 0x01,
    ping        = 0x0C,
    mode        = 0x0B,
    le_define   = 0x1A
};

/*---------------------------------------------------------------------*\
|  Skyloong Keyboard KLM Layout                                         |
\*---------------------------------------------------------------------*/
key_set keys_GK104 =
{
    {   0,      0,      0,          0,          KEY_EN_ESCAPE,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      2,          2,          KEY_EN_F1,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      3,          3,          KEY_EN_F2,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      4,          4,          KEY_EN_F3,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      5,          5,          KEY_EN_F4,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      7,          7,          KEY_EN_F5,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      8,          8,          KEY_EN_F6,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      9,          9,          KEY_EN_F7,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      10,         10,         KEY_EN_F8,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      11,         11,         KEY_EN_F9,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      12,         12,         KEY_EN_F10,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      13,         13,         KEY_EN_F11,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      14,         14,         KEY_EN_F12,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      15,         15,         KEY_EN_PRINT_SCREEN,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      16,         16,         KEY_EN_SCROLL_LOCK,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      17,         17,         KEY_EN_PAUSE_BREAK,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      0,          22,         KEY_EN_BACK_TICK,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      1,          23,         KEY_EN_1,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      2,          24,         KEY_EN_2,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      3,          25,         KEY_EN_3,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      4,          26,         KEY_EN_4,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      5,          27,         KEY_EN_5,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      6,          28,         KEY_EN_6,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      7,          29,         KEY_EN_7,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      8,          30,         KEY_EN_8,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      9,          31,         KEY_EN_9,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      10,         32,         KEY_EN_0,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      11,         33,         KEY_EN_MINUS,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      12,         34,         KEY_EN_EQUALS,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      14,         36,         KEY_EN_BACKSPACE,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      15,         37,         KEY_EN_INSERT,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      16,         38,         KEY_EN_HOME,                KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      17,         39,         KEY_EN_PAGE_UP,             KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      18,         40,         KEY_EN_NUMPAD_LOCK,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      19,         41,         KEY_EN_NUMPAD_DIVIDE,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      20,         42,         KEY_EN_NUMPAD_TIMES,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      21,         43,         KEY_EN_NUMPAD_MINUS,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      0,          44,         KEY_EN_TAB,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      1,          45,         KEY_EN_Q,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      2,          46,         KEY_EN_W,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      3,          47,         KEY_EN_E,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      4,          48,         KEY_EN_R,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      5,          49,         KEY_EN_T,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      6,          50,         KEY_EN_Y,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      7,          51,         KEY_EN_U,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      8,          52,         KEY_EN_I,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      9,          53,         KEY_EN_O,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      10,         54,         KEY_EN_P,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      11,         55,         KEY_EN_LEFT_BRACKET,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      12,         56,         KEY_EN_RIGHT_BRACKET,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      14,         58,         KEY_EN_ANSI_BACK_SLASH,     KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      15,         59,         KEY_EN_DELETE,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      16,         60,         KEY_EN_END,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      17,         61,         KEY_EN_PAGE_DOWN,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      18,         62,         KEY_EN_NUMPAD_7,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      19,         63,         KEY_EN_NUMPAD_8,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      20,         64,         KEY_EN_NUMPAD_9,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      21,         65,         KEY_EN_NUMPAD_PLUS,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      0,          66,         KEY_EN_CAPS_LOCK,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      1,          67,         KEY_EN_A,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      2,          68,         KEY_EN_S,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      3,          69,         KEY_EN_D,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      4,          70,         KEY_EN_F,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      5,          71,         KEY_EN_G,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      6,          72,         KEY_EN_H,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      7,          73,         KEY_EN_J,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      8,          74,         KEY_EN_K,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      9,          75,         KEY_EN_L,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      10,         76,         KEY_EN_SEMICOLON,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      11,         77,         KEY_EN_QUOTE,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      12,         78,         KEY_EN_POUND,               KEYBOARD_OPCODE_SWAP_ONLY  },
    {   0,      3,      13,         79,         KEY_EN_ANSI_ENTER,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      18,         84,         KEY_EN_NUMPAD_4,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      19,         85,         KEY_EN_NUMPAD_5,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      20,         86,         KEY_EN_NUMPAD_6,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      0,          88,         KEY_EN_LEFT_SHIFT,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      2,          90,         KEY_EN_Z,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      3,          91,         KEY_EN_X,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      4,          92,         KEY_EN_C,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      5,          93,         KEY_EN_V,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      6,          94,         KEY_EN_B,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      7,          95,         KEY_EN_N,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      8,          96,         KEY_EN_M,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      9,          97,         KEY_EN_COMMA,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      10,         98,         KEY_EN_PERIOD,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      11,         99,         KEY_EN_FORWARD_SLASH,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      14,         102,        KEY_EN_RIGHT_SHIFT,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      16,         104,        KEY_EN_UP_ARROW,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      18,         106,        KEY_EN_NUMPAD_1,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      19,         107,        KEY_EN_NUMPAD_2,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      20,         108,        KEY_EN_NUMPAD_3,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      21,         109,        KEY_EN_NUMPAD_ENTER,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      0,          110,        KEY_EN_LEFT_CONTROL,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      1,          111,        KEY_EN_LEFT_WINDOWS,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      2,          112,        KEY_EN_LEFT_ALT,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      4,          114,        "Key: Left Space",          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      6,          116,        KEY_EN_SPACE,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      8,          118,        "Key: Right Space",         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      10,         120,        KEY_EN_RIGHT_ALT,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      11,         121,        KEY_EN_RIGHT_FUNCTION,      KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      12,         122,        KEY_EN_MENU,                KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      14,         124,        KEY_EN_RIGHT_CONTROL,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      15,         125,        KEY_EN_LEFT_ARROW,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      16,         126,        KEY_EN_DOWN_ARROW,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      17,         127,        KEY_EN_RIGHT_ARROW,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      18,         128,        KEY_EN_NUMPAD_0,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      20,         130,        KEY_EN_NUMPAD_PERIOD,       KEYBOARD_OPCODE_SWAP_ONLY, },
};

key_set keys_GK980 =
{
    {   0,      0,      0,          0,          KEY_EN_ESCAPE,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      3,          3,          KEY_EN_F1,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      4,          4,          KEY_EN_F2,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      5,          5,          KEY_EN_F3,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      6,          6,          KEY_EN_F4,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      7,          7,          KEY_EN_F5,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      8,          8,          KEY_EN_F6,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      9,          9,          KEY_EN_F7,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      10,         10,         KEY_EN_F8,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      11,         11,         KEY_EN_F9,                  KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      12,         12,         KEY_EN_F10,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      13,         13,         KEY_EN_F11,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      14,         14,         KEY_EN_F12,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      15,         15,         KEY_EN_DELETE,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      16,         16,         KEY_EN_PAGE_UP,             KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      17,         17,         KEY_EN_PAGE_DOWN,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      18,         18,         KEY_EN_PAUSE_BREAK,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      19,         19,         KEY_EN_PRINT_SCREEN,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      0,          22,         KEY_EN_BACK_TICK,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      2,          24,         KEY_EN_1,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      3,          25,         KEY_EN_2,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      4,          26,         KEY_EN_3,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      5,          27,         KEY_EN_4,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      6,          28,         KEY_EN_5,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      7,          29,         KEY_EN_6,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      8,          30,         KEY_EN_7,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      9,          31,         KEY_EN_8,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      10,         32,         KEY_EN_9,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      11,         33,         KEY_EN_0,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      12,         34,         KEY_EN_MINUS,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      13,         35,         KEY_EN_EQUALS,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      14,         36,         KEY_EN_BACKSPACE,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      16,         38,         KEY_EN_NUMPAD_LOCK,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      17,         39,         KEY_EN_NUMPAD_DIVIDE,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      18,         40,         KEY_EN_NUMPAD_TIMES,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      19,         41,         KEY_EN_NUMPAD_MINUS,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      0,          44,         KEY_EN_TAB,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      2,          46,         KEY_EN_Q,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      3,          47,         KEY_EN_W,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      4,          48,         KEY_EN_E,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      5,          49,         KEY_EN_R,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      6,          50,         KEY_EN_T,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      7,          51,         KEY_EN_Y,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      8,          52,         KEY_EN_U,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      9,          53,         KEY_EN_I,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      10,         54,         KEY_EN_O,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      11,         55,         KEY_EN_P,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      12,         56,         KEY_EN_LEFT_BRACKET,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      13,         57,         KEY_EN_RIGHT_BRACKET,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      14,         58,         KEY_EN_ANSI_BACK_SLASH,     KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      16,         60,         KEY_EN_NUMPAD_7,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      17,         61,         KEY_EN_NUMPAD_8,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      18,         62,         KEY_EN_NUMPAD_9,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      19,         63,         KEY_EN_NUMPAD_PLUS,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      0,          66,         KEY_EN_CAPS_LOCK,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      2,          68,         KEY_EN_A,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      3,          69,         KEY_EN_S,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      4,          70,         KEY_EN_D,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      5,          71,         KEY_EN_F,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      6,          72,         KEY_EN_G,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      7,          73,         KEY_EN_H,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      8,          74,         KEY_EN_J,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      9,          75,         KEY_EN_K,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      10,         76,         KEY_EN_L,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      11,         77,         KEY_EN_SEMICOLON,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      12,         78,         KEY_EN_QUOTE,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      14,         80,         KEY_EN_ANSI_ENTER,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      16,         82,         KEY_EN_NUMPAD_4,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      17,         83,         KEY_EN_NUMPAD_5,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      18,         84,         KEY_EN_NUMPAD_6,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      0,          88,         KEY_EN_LEFT_SHIFT,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      2,          90,         KEY_EN_Z,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      3,          91,         KEY_EN_X,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      4,          92,         KEY_EN_C,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      5,          93,         KEY_EN_V,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      6,          94,         KEY_EN_B,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      7,          95,         KEY_EN_N,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      8,          96,         KEY_EN_M,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      9,          97,         KEY_EN_COMMA,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      10,         98,         KEY_EN_PERIOD,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      11,         99,         KEY_EN_FORWARD_SLASH,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      13,         101,        KEY_EN_RIGHT_SHIFT,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      15,         103,        KEY_EN_UP_ARROW,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      16,         104,        KEY_EN_NUMPAD_1,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      17,         105,        KEY_EN_NUMPAD_2,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      18,         106,        KEY_EN_NUMPAD_3,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      19,         107,        KEY_EN_NUMPAD_ENTER,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      0,          110,        KEY_EN_LEFT_CONTROL,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      1,          111,        KEY_EN_LEFT_WINDOWS,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      2,          112,        KEY_EN_LEFT_ALT,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      4,          114,        "Key: Left Space",          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      6,          116,        KEY_EN_SPACE,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      7,          116,        "Key: Middle Space",        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      8,          118,        "Key: Right Space",         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      10,         120,        KEY_EN_RIGHT_ALT,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      11,         121,        KEY_EN_RIGHT_FUNCTION,      KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      12,         122,        KEY_EN_RIGHT_CONTROL,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      14,         124,        KEY_EN_LEFT_ARROW,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      15,         125,        KEY_EN_DOWN_ARROW,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      16,         126,        KEY_EN_RIGHT_ARROW,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      17,         127,        KEY_EN_NUMPAD_0,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      5,      18,         128,        KEY_EN_NUMPAD_PERIOD,       KEYBOARD_OPCODE_SWAP_ONLY, },
};

key_set keys_GK61 =
{
    {   0,      0,      0,          0,          KEY_EN_BACK_TICK,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      2,          2,          KEY_EN_1,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      3,          3,          KEY_EN_2,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      4,          4,          KEY_EN_3,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      5,          5,          KEY_EN_4,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      6,          6,          KEY_EN_5,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      7,          7,          KEY_EN_6,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      8,          8,          KEY_EN_7,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      9,          9,          KEY_EN_8,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      10,         10,         KEY_EN_9,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      11,         11,         KEY_EN_0,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      12,         12,         KEY_EN_MINUS,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      13,         13,         KEY_EN_EQUALS,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      0,      14,         14,         KEY_EN_BACKSPACE,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      0,          22,         KEY_EN_TAB,                 KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      2,          24,         KEY_EN_Q,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      3,          25,         KEY_EN_W,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      4,          26,         KEY_EN_E,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      5,          27,         KEY_EN_R,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      6,          28,         KEY_EN_T,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      7,          29,         KEY_EN_Y,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      8,          30,         KEY_EN_U,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      9,          31,         KEY_EN_I,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      10,         32,         KEY_EN_O,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      11,         33,         KEY_EN_P,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      12,         34,         KEY_EN_LEFT_BRACKET,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      13,         35,         KEY_EN_RIGHT_BRACKET,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      1,      14,         36,         KEY_EN_ANSI_BACK_SLASH,     KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      0,          44,         KEY_EN_CAPS_LOCK,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      2,          46,         KEY_EN_A,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      3,          47,         KEY_EN_S,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      4,          48,         KEY_EN_D,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      5,          49,         KEY_EN_F,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      6,          50,         KEY_EN_G,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      7,          51,         KEY_EN_H,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      8,          52,         KEY_EN_J,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      9,          53,         KEY_EN_K,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      10,         54,         KEY_EN_L,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      11,         55,         KEY_EN_SEMICOLON,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      12,         56,         KEY_EN_QUOTE,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      2,      14,         58,         KEY_EN_ANSI_ENTER,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      0,          66,         KEY_EN_LEFT_SHIFT,          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      2,          68,         KEY_EN_Z,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      3,          69,         KEY_EN_X,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      4,          70,         KEY_EN_C,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      5,          71,         KEY_EN_V,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      6,          72,         KEY_EN_B,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      7,          73,         KEY_EN_N,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      8,          74,         KEY_EN_M,                   KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      9,          75,         KEY_EN_COMMA,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      10,         76,         KEY_EN_PERIOD,              KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      11,         77,         KEY_EN_FORWARD_SLASH,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      3,      14,         80,         KEY_EN_RIGHT_SHIFT,         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      0,          88,         KEY_EN_LEFT_CONTROL,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      1,          89,         KEY_EN_LEFT_WINDOWS,        KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      2,          90,         KEY_EN_LEFT_ALT,            KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      4,          92,         "Key: Left Space",          KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      7,          95,         KEY_EN_SPACE,               KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      9,          97,         "Key: Right Space",         KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      10,         98,         KEY_EN_RIGHT_ALT,           KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      11,         99,         KEY_EN_MENU,                KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      13,         101,        KEY_EN_RIGHT_CONTROL,       KEYBOARD_OPCODE_SWAP_ONLY, },
    {   0,      4,      14,         102,        KEY_EN_RIGHT_FUNCTION,      KEYBOARD_OPCODE_SWAP_ONLY, },
};

static gk_model gk_models[] = {
    { 656802051, "Skyloong GK104 Pro", keys_GK104 },
    { 656802008, "Skyloong GK980", keys_GK980 },
    { 656802031, "Skyloong GK61", keys_GK61 },
    { 656802032, "Skyloong GK61", keys_GK61 }
};

SkyloongKeyboardController::SkyloongKeyboardController(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;
    model       = NULL;

    SendCommand(command::info, INFO_MODEL_ID);

    unsigned char buf[PACKET_SIZE];
    hid_read(dev, buf, PACKET_SIZE);
    uint32_t model_id = (((uint32_t) buf[11]) << 24) | (buf[10] << 16) | (buf[9] << 8) | (buf[8]);
    for(unsigned int model_idx = 0; model_idx < (sizeof(gk_models) / sizeof(gk_models[0])); model_idx++)
    {
        if(model_id == gk_models[model_idx].id)
        {
            model = &(gk_models[model_idx]);
            break;
        }
    }

    SendCommand(command::ping, SUBCOMMAND_NONE);
    SendCommand(command::mode, MODE_ONLINE);
    SendCommand(command::ping, SUBCOMMAND_NONE);
}

SkyloongKeyboardController::~SkyloongKeyboardController()
{
    SendCommand(command::mode, MODE_OFFLINE);
    hid_close(dev);
}

std::string SkyloongKeyboardController::GetDeviceLocation()
{
    return("HID: " + location);
}

gk_model* SkyloongKeyboardController::GetModel()
{
    return model;
}

void SkyloongKeyboardController::Ping()
{
    SendCommand(command::ping, SUBCOMMAND_NONE);
}

void SkyloongKeyboardController::SendCommand(char command, char sub_command)
{
    unsigned char buf[PACKET_SIZE];
    memset(buf, 0x00, PACKET_SIZE);

    buf[0x01]       = command;
    buf[0x02]       = sub_command;

    uint16_t crc    = Crc16CcittFalse(buf+1, PACKET_SIZE-1);

    buf[0x07]       = crc & 0xFF;
    buf[0x08]       = crc >> 8;

    hid_write(dev, buf, PACKET_SIZE);

    std::this_thread::sleep_for(1ms);
}

void SkyloongKeyboardController::SendColorPacket(std::vector<RGBColor> colors, std::vector<led> *leds, int brightness)
{
    unsigned char le_data[TOTAL_LED_BYTES];
    memset(le_data, 0x00, TOTAL_LED_BYTES);

    for(unsigned int i = 0; i < leds->size(); i++)
    {
        int index           = leds->at(i).value * 4;
        le_data[index++]    = RGBGetRValue(colors[i]);
        le_data[index++]    = RGBGetGValue(colors[i]);
        le_data[index++]    = RGBGetBValue(colors[i]);
        le_data[index++]    = brightness;
    }

    for(int n = 0; n < TOTAL_LED_BYTES; n += LED_BYTES_IN_CHUNK) {
        if(n + LED_BYTES_IN_CHUNK <= TOTAL_LED_BYTES)
        {
            SetLEDefine(n, &le_data[n], LED_BYTES_IN_CHUNK);
        }
        else
        {
            SetLEDefine(n, &le_data[n], TOTAL_LED_BYTES - n);
        }
    }

    SendCommand(command::le_define, LE_DEFINE_SAVE);
}

void SkyloongKeyboardController::SetLEDefine(int address, unsigned char *le_data, int le_data_length)
{
    unsigned char buf[PACKET_SIZE];
    memset(buf, 0x00, PACKET_SIZE);

    buf[0x01]       = command::le_define;
    buf[0x02]       = LE_DEFINE_SET;

    int header      = (address + ((le_data_length << 24) & 0xFF000000)) | 0;
    buf[0x03]       = header & 0xFF;
    buf[0x04]       = (header >> 8) & 0xFF;
    buf[0x05]       = (header >> 16) & 0xFF;
    buf[0x06]       = (header >> 24) & 0xFF;

    std::copy(le_data, le_data + le_data_length, buf + 9);

    uint16_t crc    = Crc16CcittFalse(buf+1, PACKET_SIZE-1);
    buf[0x07]       = crc & 0xFF;
    buf[0x08]       = crc >> 8;

    hid_write(dev, buf, PACKET_SIZE);

    std::this_thread::sleep_for(1ms);
}

uint16_t SkyloongKeyboardController::Crc16CcittFalse(const uint8_t *buffer, uint16_t size)
{
    uint16_t crc = 0xFFFF;

    while(size--)
    {
        crc ^= (*buffer++ << 8);

        for(uint8_t i = 0; i < 8; ++i)
        {
            if(crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }
        }
    }

    return crc;
}
