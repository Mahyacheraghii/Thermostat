#pragma once

// Prefer real vendor libraries when available; fall back to lightweight
// shims so the IDE/clangd doesn't report missing include errors.

#if defined(__has_include)
#if __has_include(<XPT2046_Touchscreen.h>)
#include <XPT2046_Touchscreen.h>
#define TOUCH_DRV_HAVE_XPT2046 1
#endif
#endif

#ifndef TOUCH_DRV_HAVE_XPT2046
// Minimal shim for XPT2046_Touchscreen types used by the project.
#ifndef TOUCH_DRIVERS_TS_POINT_DEFINED
#define TOUCH_DRIVERS_TS_POINT_DEFINED
struct TS_Point
{
    int x;
    int y;
    int z;
};
#endif
class XPT2046_Touchscreen
{
public:
    XPT2046_Touchscreen(int cs, int irq) {}
    bool begin() { return true; }
    bool touched() { return false; }
    TS_Point getPoint() { return TS_Point{0, 0, 0}; }
};
#endif
