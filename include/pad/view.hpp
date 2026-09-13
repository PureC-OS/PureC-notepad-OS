#pragma once
#include <stdint.h>
#include "gui/win.hpp"

struct PadGeo {
    uint32_t cw;
    uint32_t ch;
    uint32_t rows;
    uint32_t cols;
    uint32_t tx;
    uint32_t ty;
    uint32_t tw;
    uint32_t th;
    uint32_t tab_w;
    uint32_t tab_n;
    uint32_t brow_ly;
    uint32_t sb_x;
    uint32_t sb_y;
    uint32_t sb_h;
    uint32_t th_y;
    uint32_t th_h;
};

PadGeo pad_geo(NpWin* w);
bool pad_draw(NpWin* w);
uint32_t pad_click_toolbar(PadGeo* g, int32_t cx, int32_t cy);
int32_t pad_click_tab(PadGeo* g, int32_t cx, int32_t cy);
int32_t pad_click_browser(PadGeo* g, int32_t cx, int32_t cy);
bool pad_click_text(PadGeo* g, int32_t cx, int32_t cy, uint32_t* line, uint32_t* col);
bool pad_click_scroll(PadGeo* g, int32_t cx, int32_t cy, int* dir);
