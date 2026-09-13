#pragma once
#include <stdint.h>
#include "pad/cfg.hpp"

struct PadEntry {
    char name[64];
    bool dir;
    uint32_t size;
};

extern char pad_dir[PAD_PATH_CAP];
extern PadEntry pad_entries[PAD_BROW_MAX];
extern int32_t pad_entry_count;
extern int32_t pad_entry_top;
extern int32_t pad_entry_sel;
extern uint32_t pad_brow_rows;

void pad_dir_norm(char* out, uint32_t cap, const char* src);
bool pad_dir_join(char* out, uint32_t cap, const char* dir, const char* name);
void pad_dir_up(char* out, uint32_t cap, const char* dir);
void pad_brow_refresh();
