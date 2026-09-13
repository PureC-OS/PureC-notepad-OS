#pragma once
#include <stdint.h>
#include "sys/np_sys.h"

#define NP_EV_NONE 0
#define NP_EV_KEY 1
#define NP_EV_MOUSE_MOVE 2
#define NP_EV_MOUSE_DOWN 3
#define NP_EV_MOUSE_UP 4
#define NP_EV_MOVE 5
#define NP_EV_MIN 6
#define NP_EV_FOCUS 7
#define NP_EV_REPAINT 8
#define NP_EV_SPECIAL 9
#define NP_EV_CLOSE 10

struct NpRect {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
};

struct NpEvent {
    uint32_t type;
    int32_t x;
    int32_t y;
    int32_t key;
    uint8_t button;
};

struct NpWin {
    NpRect frame;
    NpRect client;
    char title[64];
    int32_t pmx;
    int32_t pmy;
    int32_t dox;
    int32_t doy;
    uint8_t pbtn;
    bool drag;
    bool min;
    bool open;
    bool focus;
    bool reg;
    bool repaint;
};

bool np_win_init(NpWin* w, const char* title, uint32_t x, uint32_t y, uint32_t ww, uint32_t hh);
bool np_win_center(NpWin* w, const char* title, uint32_t ww, uint32_t hh);
void np_win_begin(NpWin* w);
void np_win_end(NpWin* w);
void np_win_close(NpWin* w);
bool np_win_move(NpWin* w, uint32_t x, uint32_t y);
bool np_win_opened(const NpWin* w);
bool np_win_minimized(const NpWin* w);
NpRect np_win_client(const NpWin* w);
void np_win_clear(NpWin* w, uint32_t color);
void np_win_rect(NpWin* w, NpRect b, uint32_t color);
void np_win_text(NpWin* w, uint32_t x, uint32_t y, const char* s, uint32_t color);
bool np_win_poll(NpWin* w, NpEvent* e);
bool np_point_in(int32_t x, int32_t y, NpRect r);
