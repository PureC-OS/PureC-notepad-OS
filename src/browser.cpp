#include "pad/browser.hpp"
#include "com/str.hpp"
#include "sys/np_os.hpp"

char pad_dir[PAD_PATH_CAP];
PadEntry pad_entries[PAD_BROW_MAX];
int32_t pad_entry_count = 0;
int32_t pad_entry_top = 0;
int32_t pad_entry_sel = 0;
uint32_t pad_brow_rows = 1;

void pad_dir_norm(char* out, uint32_t cap, const char* src) {
    if (!out || !cap) return;
    if (!src || !*src) { np_cpy(out, "/", cap); return; }
    uint32_t n = np_strlen(src);
    while (n > 1 && src[n - 1] == '/') n--;
    if (n + 1 > cap) n = cap - 1;
    for (uint32_t i = 0; i < n; i++) out[i] = src[i];
    out[n] = 0;
    if (!out[0]) np_cpy(out, "/", cap);
}

bool pad_dir_join(char* out, uint32_t cap, const char* dir, const char* name) {
    if (!out || !cap || !dir || !name) return false;
    uint32_t dl = np_strlen(dir);
    uint32_t nl = np_strlen(name);
    uint32_t need = dl + (dl > 1 ? 1 : 0) + nl + 1;
    if (need > cap) return false;
    np_cpy(out, dir, cap);
    uint32_t p = dl;
    if (dl > 1) { out[p++] = '/'; }
    for (uint32_t i = 0; i < nl; i++) out[p++] = name[i];
    out[p] = 0;
    return true;
}

void pad_dir_up(char* out, uint32_t cap, const char* dir) {
    if (!out || !cap || !dir) return;
    uint32_t n = np_strlen(dir);
    while (n > 1 && dir[n - 1] == '/') n--;
    while (n > 1 && dir[n - 1] != '/') n--;
    if (n == 0) { np_cpy(out, "/", cap); return; }
    if (n + 1 > cap) n = cap - 1;
    for (uint32_t i = 0; i < n; i++) out[i] = dir[i];
    out[n] = 0;
    if (n > 1 && out[n - 1] == '/') out[n - 1] = 0;
    if (!out[0]) np_cpy(out, "/", cap);
}

static void pad_brow_sort() {
    for (int32_t i = 0; i < pad_entry_count; i++) {
        for (int32_t j = i + 1; j < pad_entry_count; j++) {
            bool swap = false;
            if (pad_entries[j].dir && !pad_entries[i].dir) swap = true;
            else if (pad_entries[j].dir == pad_entries[i].dir && np_cmp(pad_entries[j].name, pad_entries[i].name) < 0) swap = true;
            if (swap) {
                PadEntry t = pad_entries[i];
                pad_entries[i] = pad_entries[j];
                pad_entries[j] = t;
            }
        }
    }
}

void pad_brow_refresh() {
    pad_entry_count = 0;
    pad_entry_top = 0;
    pad_entry_sel = 0;
    NpDirEnt raw[PAD_BROW_MAX];
    int32_t n = np_list_dir(pad_dir, raw, PAD_BROW_MAX);
    if (n <= 0) return;
    if (n > (int32_t)PAD_BROW_MAX) n = PAD_BROW_MAX;
    for (int32_t i = 0; i < n; i++) {
        np_cpy(pad_entries[i].name, raw[i].name, sizeof(pad_entries[i].name));
        pad_entries[i].dir = (raw[i].attr & NP_ATTR_DIR) != 0;
        pad_entries[i].size = raw[i].size;
    }
    pad_entry_count = n;
    pad_brow_sort();
}
