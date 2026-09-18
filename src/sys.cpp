#include "sys/np_os.hpp"
#include "sys/np_font.hpp"

void np_sleep(uint32_t ms) {
    np_sys3(NP_SYS_SLEEP, ms, 0, 0);
}

void np_exit(int32_t code) {
    np_sys3(NP_SYS_EXIT, (uint64_t)(int64_t)code, 0, 0);
    for (;;) { __asm__ volatile("pause"); }
}

int32_t np_try_char() {
    return (int32_t)np_sys3(NP_SYS_TRY_GETCHAR, 0, 0, 0);
}

int32_t np_try_special() {
    return (int32_t)np_sys3(NP_SYS_TRY_GET_SPECIAL, 0, 0, 0);
}

int32_t np_cmdline(char* buf, uint32_t cap) {
    return (int32_t)np_sys3(NP_SYS_GET_COMMAND_LINE, (uint64_t)buf, cap, 0);
}

int32_t np_env_get(const char* name, char* buf, uint32_t cap) {
    return (int32_t)np_sys3(NP_SYS_ENV_GET, (uint64_t)name, (uint64_t)buf, cap);
}

int32_t np_open(const char* path) {
    return (int32_t)np_sys3(NP_SYS_FILE_OPEN, (uint64_t)path, 0, 0);
}

int32_t np_read(int32_t fd, void* buf, uint32_t cap) {
    return (int32_t)np_sys3(NP_SYS_FILE_READ, (uint32_t)fd, (uint64_t)buf, cap);
}

int32_t np_close(int32_t fd) {
    return (int32_t)np_sys3(NP_SYS_FILE_CLOSE, (uint32_t)fd, 0, 0);
}

int32_t np_write_file(const char* path, const void* buf, uint32_t size) {
    return (int32_t)np_sys3(NP_SYS_FILE_WRITE, (uint64_t)path, (uint64_t)buf, size);
}

int32_t np_create_file(const char* path) {
    return (int32_t)np_sys3(NP_SYS_FILE_CREATE, (uint64_t)path, 0, 0);
}

int32_t np_list_dir(const char* path, NpDirEnt* out, uint32_t cap) {
    return (int32_t)np_sys3(NP_SYS_DIR_LIST_LONG, (uint64_t)path, (uint64_t)out, cap);
}

NpDisplay np_display() {
    NpDisplay d;
    d.width = 0;
    d.height = 0;
    d.ok = false;
    NpFbInfo f;
    f.width = 0;
    f.height = 0;
    f.available = 0;
    if (np_sys3(NP_SYS_FB_INFO, (uint64_t)&f, 0, 0) < 0) return d;
    if (!f.available || !f.width || !f.height) return d;
    d.width = f.width;
    d.height = f.height;
    d.ok = true;
    return d;
}

void np_begin_frame() {
    np_sys3(NP_SYS_FB_BEGIN_UPDATE, 0, 0, 0);
}

void np_end_frame() {
    np_sys3(NP_SYS_FB_END_UPDATE, 0, 0, 0);
}

void np_redraw_desktop() {
    np_sys3(NP_SYS_DESKTOP_REDRAW, 0, 0, 0);
}

void np_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    if (!w || !h) return;
    np_sys5(NP_SYS_DRAW_RECT, x, y, w, h, color);
}

void np_text(uint32_t x, uint32_t y, const char* s, uint32_t fg, uint32_t bg) {
    (void)bg;
    if (!s) return;
    for (; *s; ++s) {
        uint8_t ch = (uint8_t)*s;
        if (ch >= 128) { x += 8; continue; }
        if (ch < 32) { x += 8; continue; }
        const uint8_t* g = NP_FONT[ch];
        for (uint32_t row = 0; row < 8; row++) {
            uint8_t bits = g[row];
            if (!bits) continue;
            uint32_t run_start = 0;
            bool in_run = false;
            for (uint32_t col = 0; col <= 8; col++) {
                bool set = col < 8 && (bits & (1u << (7 - col)));
                if (set && !in_run) { run_start = col; in_run = true; }
                if ((!set || col == 8) && in_run) {
                    np_rect(x + run_start, y + row, col - run_start, 1, fg);
                    in_run = false;
                }
            }
        }
        x += 8;
    }
}

bool np_mouse(NpMouse* m) {
    if (!m) return false;
    return np_sys3(NP_SYS_GET_MOUSE, (uint64_t)m, 0, 0) >= 0;
}

bool np_win_register(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    NpWinReq r;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    return np_sys3(NP_SYS_WIN_REGISTER, (uint64_t)&r, 0, 0) >= 0;
}

bool np_win_update(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    NpWinReq r;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    return np_sys3(NP_SYS_WIN_UPDATE, (uint64_t)&r, 0, 0) >= 0;
}

void np_win_unregister() {
    np_sys3(NP_SYS_WIN_UNREGISTER, 0, 0, 0);
}

uint32_t np_win_state() {
    int64_t s = np_sys3(NP_SYS_WIN_STATE, 0, 0, 0);
    return s < 0 ? 0 : (uint32_t)s;
}

void np_win_repaint_done() {
    np_sys3(NP_SYS_WIN_REPAINT_DONE, 0, 0, 0);
}
