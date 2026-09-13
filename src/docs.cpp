#include "pad/docs.hpp"
#include "com/str.hpp"

static char pad_arena[PAD_MAX_TABS * PAD_TAB_CAP];

PadDoc pad_docs[PAD_MAX_TABS];
int32_t pad_active = -1;
char pad_msg[PAD_STATUS_CAP];
bool pad_msg_err = false;
bool pad_exit_armed = false;
uint32_t pad_prompt = PAD_PR_NONE;
char pad_prompt_buf[PAD_PROMPT_CAP];

void pad_set_msg(const char* s, bool err) {
    np_cpy(pad_msg, s ? s : "", sizeof(pad_msg));
    pad_msg_err = err;
}

PadDoc* pad_cur() {
    if (pad_active < 0 || pad_active >= (int32_t)PAD_MAX_TABS) return 0;
    if (!pad_docs[pad_active].used) return 0;
    return &pad_docs[pad_active];
}

int32_t pad_count() {
    int32_t n = 0;
    for (uint32_t i = 0; i < PAD_MAX_TABS; i++) if (pad_docs[i].used) n++;
    return n;
}

bool pad_docs_init() {
    for (uint32_t i = 0; i < PAD_MAX_TABS; i++) {
        pad_docs[i].used = false;
        pad_docs[i].dirty = false;
        pad_docs[i].path[0] = 0;
        pad_docs[i].data = pad_arena + i * PAD_TAB_CAP;
        pad_docs[i].len = 0;
        pad_docs[i].cursor = 0;
        pad_docs[i].vline = 0;
        pad_docs[i].vcol = 0;
    }
    pad_active = -1;
    pad_msg[0] = 0;
    pad_msg_err = false;
    pad_exit_armed = false;
    pad_prompt = PAD_PR_NONE;
    pad_prompt_buf[0] = 0;
    return pad_new_doc() >= 0;
}

int32_t pad_new_doc() {
    for (uint32_t i = 0; i < PAD_MAX_TABS; i++) {
        if (!pad_docs[i].used) {
            pad_docs[i].used = true;
            pad_docs[i].dirty = false;
            pad_docs[i].path[0] = 0;
            pad_docs[i].len = 0;
            pad_docs[i].cursor = 0;
            pad_docs[i].vline = 0;
            pad_docs[i].vcol = 0;
            pad_active = (int32_t)i;
            return (int32_t)i;
        }
    }
    return -1;
}

bool pad_close_doc(int32_t idx) {
    if (idx < 0 || idx >= (int32_t)PAD_MAX_TABS || !pad_docs[idx].used) return false;
    pad_docs[idx].used = false;
    pad_docs[idx].dirty = false;
    pad_docs[idx].len = 0;
    if (pad_count() == 0) {
        pad_new_doc();
        return true;
    }
    if (pad_active == idx) {
        for (int32_t i = idx; i >= 0; i--) {
            if (pad_docs[i].used) { pad_active = i; return true; }
        }
        for (uint32_t i = 0; i < PAD_MAX_TABS; i++) {
            if (pad_docs[i].used) { pad_active = (int32_t)i; return true; }
        }
    }
    return true;
}

bool pad_any_dirty() {
    for (uint32_t i = 0; i < PAD_MAX_TABS; i++) {
        if (pad_docs[i].used && pad_docs[i].dirty) return true;
    }
    return false;
}

uint32_t pad_lstart(PadDoc* t, uint32_t off) {
    if (!t) return 0;
    if (off > t->len) off = t->len;
    while (off && t->data[off - 1] != '\n') off--;
    return off;
}

uint32_t pad_lend(PadDoc* t, uint32_t off) {
    if (!t) return 0;
    if (off > t->len) off = t->len;
    while (off < t->len && t->data[off] != '\n') off++;
    return off;
}

uint32_t pad_lnum(PadDoc* t, uint32_t off) {
    uint32_t n = 0;
    if (!t) return 0;
    if (off > t->len) off = t->len;
    for (uint32_t i = 0; i < off; i++) if (t->data[i] == '\n') n++;
    return n;
}

uint32_t pad_loff(PadDoc* t, uint32_t line) {
    uint32_t n = 0;
    if (!t) return 0;
    for (uint32_t i = 0; i < t->len; i++) {
        if (n == line) return i;
        if (t->data[i] == '\n') n++;
    }
    return t->len;
}

uint32_t pad_lines(PadDoc* t) {
    if (!t) return 1;
    return pad_lnum(t, t->len) + 1;
}

uint32_t pad_vis_rows(uint32_t ch) {
    uint32_t top = PAD_TOOL_H + PAD_TABS_H;
    if (ch <= top + PAD_STAT_H + PAD_ROW_H) return 1;
    return (ch - top - PAD_STAT_H) / PAD_ROW_H;
}

uint32_t pad_vis_cols(uint32_t cw) {
    uint32_t left = PAD_BROW_W + PAD_GUT_W;
    if (cw <= left + PAD_COL_W * 2) return 8;
    return (cw - left) / PAD_COL_W;
}

void pad_show_cursor(PadDoc* t, uint32_t rows, uint32_t cols) {
    if (!t) return;
    uint32_t ln = pad_lnum(t, t->cursor);
    uint32_t col = t->cursor - pad_lstart(t, t->cursor);
    if (ln < t->vline) t->vline = ln;
    else if (ln >= t->vline + rows) t->vline = ln - rows + 1;
    if (col < t->vcol) t->vcol = col;
    else if (col >= t->vcol + cols) t->vcol = col - cols + 1;
}

bool pad_put(PadDoc* t, char c) {
    if (!t || t->len >= PAD_TAB_CAP) return false;
    if (t->cursor > t->len) t->cursor = t->len;
    for (uint32_t i = t->len; i > t->cursor; i--) t->data[i] = t->data[i - 1];
    t->data[t->cursor] = c;
    t->cursor++;
    t->len++;
    t->dirty = true;
    return true;
}

bool pad_newline(PadDoc* t) {
    if (!t) return false;
    uint32_t s = pad_lstart(t, t->cursor);
    uint32_t ind = 0;
    while (s + ind < t->len && (t->data[s + ind] == ' ' || t->data[s + ind] == '\t')) ind++;
    if (!pad_put(t, '\n')) return false;
    for (uint32_t i = 0; i < ind; i++) {
        if (!pad_put(t, t->data[s + i])) break;
    }
    return true;
}

bool pad_tab(PadDoc* t) {
    if (!t) return false;
    for (int i = 0; i < 4; i++) {
        if (!pad_put(t, ' ')) return false;
    }
    return true;
}

bool pad_back(PadDoc* t) {
    if (!t || !t->cursor) return false;
    if (t->cursor > t->len) t->cursor = t->len;
    for (uint32_t i = t->cursor - 1; i + 1 < t->len; i++) t->data[i] = t->data[i + 1];
    t->cursor--;
    t->len--;
    t->dirty = true;
    return true;
}

bool pad_del(PadDoc* t) {
    if (!t || t->cursor >= t->len) return false;
    for (uint32_t i = t->cursor; i + 1 < t->len; i++) t->data[i] = t->data[i + 1];
    t->len--;
    t->dirty = true;
    return true;
}

void pad_vmove(PadDoc* t, int32_t n) {
    if (!t) return;
    uint32_t ln = pad_lnum(t, t->cursor);
    uint32_t col = t->cursor - pad_lstart(t, t->cursor);
    int64_t want = (int64_t)ln + n;
    if (want < 0) want = 0;
    uint32_t last = pad_lines(t) - 1;
    if ((uint64_t)want > last) want = (int64_t)last;
    uint32_t s = pad_loff(t, (uint32_t)want);
    uint32_t e = pad_lend(t, s);
    t->cursor = s + (col > e - s ? e - s : col);
}

static bool pad_match_at(PadDoc* t, uint32_t pos, const char* needle, uint32_t nl) {
    if (pos + nl > t->len) return false;
    for (uint32_t i = 0; i < nl; i++) {
        if (t->data[pos + i] != needle[i]) return false;
    }
    return true;
}

bool pad_find_next(PadDoc* t, const char* needle, bool* wrapped) {
    if (!t || !needle || !*needle) return false;
    if (wrapped) *wrapped = false;
    uint32_t nl = np_strlen(needle);
    if (!nl || nl > t->len) return false;
    uint32_t start = t->cursor + 1;
    if (start > t->len) start = 0;
    for (uint32_t i = start; i + nl <= t->len; i++) {
        if (pad_match_at(t, i, needle, nl)) { t->cursor = i; return true; }
    }
    for (uint32_t i = 0; i + nl <= t->len && i < start; i++) {
        if (pad_match_at(t, i, needle, nl)) {
            t->cursor = i;
            if (wrapped) *wrapped = true;
            return true;
        }
    }
    return false;
}
