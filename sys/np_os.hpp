#pragma once
#include "sys/np_sys.h"

struct NpDisplay {
    uint32_t width;
    uint32_t height;
    bool ok;
};

void np_sleep(uint32_t ms);
void np_exit(int32_t code);
int32_t np_try_char();
int32_t np_try_special();
int32_t np_cmdline(char* buf, uint32_t cap);
int32_t np_env_get(const char* name, char* buf, uint32_t cap);
int32_t np_open(const char* path);
int32_t np_read(int32_t fd, void* buf, uint32_t cap);
int32_t np_close(int32_t fd);
int32_t np_write_file(const char* path, const void* buf, uint32_t size);
int32_t np_create_file(const char* path);
int32_t np_list_dir(const char* path, NpDirEnt* out, uint32_t cap);
NpDisplay np_display();
void np_begin_frame();
void np_end_frame();
void np_redraw_desktop();
void np_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void np_text(uint32_t x, uint32_t y, const char* s, uint32_t fg, uint32_t bg);
bool np_mouse(NpMouse* m);
bool np_win_register(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
bool np_win_update(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void np_win_unregister();
uint32_t np_win_state();
void np_win_repaint_done();
