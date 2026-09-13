#pragma once
#include <stdint.h>

#define NP_SYS_SLEEP 3
#define NP_SYS_EXIT 60
#define NP_SYS_DRAW_RECT 100
#define NP_SYS_GET_MOUSE 102
#define NP_SYS_FB_INFO 103
#define NP_SYS_DRAW_TEXT 104
#define NP_SYS_FB_BEGIN_UPDATE 109
#define NP_SYS_FB_END_UPDATE 110
#define NP_SYS_FILE_OPEN 200
#define NP_SYS_FILE_READ 201
#define NP_SYS_FILE_CREATE 206
#define NP_SYS_FILE_WRITE 211
#define NP_SYS_FILE_CLOSE 222
#define NP_SYS_TRY_GETCHAR 227
#define NP_SYS_GET_COMMAND_LINE 239
#define NP_SYS_ENV_GET 240
#define NP_SYS_DESKTOP_REDRAW 250
#define NP_SYS_WIN_REGISTER 251
#define NP_SYS_WIN_UPDATE 252
#define NP_SYS_WIN_UNREGISTER 253
#define NP_SYS_WIN_STATE 254
#define NP_SYS_WIN_REPAINT_DONE 255
#define NP_SYS_TRY_GET_SPECIAL 258
#define NP_SYS_DIR_LIST_LONG 288

#define NP_WIN_FOCUSED 1
#define NP_WIN_REPAINT 2

#define NP_ATTR_DIR 0x10

#define NP_KEY_F1 1
#define NP_KEY_F2 2
#define NP_KEY_F3 3
#define NP_KEY_HOME 4
#define NP_KEY_END 5
#define NP_KEY_PGUP 6
#define NP_KEY_PGDN 7
#define NP_KEY_LEFT 8
#define NP_KEY_RIGHT 9
#define NP_KEY_UP 10
#define NP_KEY_DOWN 11
#define NP_KEY_DEL 12

struct NpFbInfo {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint64_t size_bytes;
    uint8_t bpp;
    uint8_t available;
    char proto[16];
};

struct NpTextReq {
    uint32_t x;
    uint32_t y;
    const char* text;
    uint32_t fg;
    uint32_t bg;
    uint32_t size;
};

struct NpWinReq {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

struct NpMouse {
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;
    uint8_t buttons;
    uint8_t has_data;
    uint8_t reserved[2];
};

struct NpDirEnt {
    char name[256];
    uint32_t size;
    uint8_t attr;
    uint8_t reserved[3];
};

static inline int64_t np_sys3(uint64_t n, uint64_t a, uint64_t b, uint64_t c) {
    int64_t r;
    __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a), "c"(b), "d"(c) : "r10", "r8", "memory");
    return r;
}

static inline int64_t np_sys5(uint64_t n, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    int64_t r;
    __asm__ volatile("int $0x80" : "=a"(r) : "a"(n), "b"(a), "c"(b), "d"(c), "S"(d), "D"(e) : "r10", "r8", "memory");
    return r;
}
