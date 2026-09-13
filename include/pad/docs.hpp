#pragma once
#include <stdint.h>
#include "pad/cfg.hpp"

struct PadDoc {
    bool used;
    bool dirty;
    char path[PAD_PATH_CAP];
    char* data;
    uint32_t len;
    uint32_t cursor;
    uint32_t vline;
    uint32_t vcol;
};

extern PadDoc pad_docs[PAD_MAX_TABS];
extern int32_t pad_active;
extern char pad_msg[PAD_STATUS_CAP];
extern bool pad_msg_err;
extern bool pad_exit_armed;
extern uint32_t pad_prompt;
extern char pad_prompt_buf[PAD_PROMPT_CAP];

void pad_set_msg(const char* s, bool err);
PadDoc* pad_cur();
int32_t pad_count();
bool pad_docs_init();
int32_t pad_new_doc();
bool pad_close_doc(int32_t idx);
bool pad_any_dirty();

uint32_t pad_lstart(PadDoc* t, uint32_t off);
uint32_t pad_lend(PadDoc* t, uint32_t off);
uint32_t pad_lnum(PadDoc* t, uint32_t off);
uint32_t pad_loff(PadDoc* t, uint32_t line);
uint32_t pad_lines(PadDoc* t);
uint32_t pad_vis_rows(uint32_t ch);
uint32_t pad_vis_cols(uint32_t cw);
void pad_show_cursor(PadDoc* t, uint32_t rows, uint32_t cols);
bool pad_put(PadDoc* t, char c);
bool pad_newline(PadDoc* t);
bool pad_tab(PadDoc* t);
bool pad_back(PadDoc* t);
bool pad_del(PadDoc* t);
void pad_vmove(PadDoc* t, int32_t n);
bool pad_find_next(PadDoc* t, const char* needle, bool* wrapped);
