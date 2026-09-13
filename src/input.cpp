#include "pad/input.hpp"
#include "pad/docs.hpp"
#include "pad/files.hpp"
#include "pad/browser.hpp"
#include "pad/view.hpp"
#include "pad/cfg.hpp"
#include "com/str.hpp"
#include "sys/np_os.hpp"
#include "sys/np_sys.h"

NpWin pad_win;

static bool pad_quit = false;
static bool pad_close_armed = false;
static char pad_last_find[PAD_PROMPT_CAP];

static void pad_sync_browser_to_doc(PadDoc* t) {
    if (!t || !t->path[0]) return;
    char tmp[PAD_PATH_CAP];
    np_cpy(tmp, t->path, sizeof(tmp));
    uint32_t n = np_strlen(tmp);
    for (int32_t i = (int32_t)n - 1; i >= 0; i--) {
        if (tmp[i] == '/') {
            tmp[i] = 0;
            if (!tmp[0]) np_cpy(tmp, "/", sizeof(tmp));
            break;
        }
    }
    pad_dir_norm(pad_dir, sizeof(pad_dir), tmp);
    pad_brow_refresh();
}

static void pad_open_path(const char* raw) {
    char full[PAD_PATH_CAP];
    pad_resolve(raw, full, sizeof(full));
    if (!full[0]) { pad_set_msg("Bad path", true); return; }
    PadDoc* cur = pad_cur();
    PadDoc* dst = 0;
    if (cur && !cur->dirty && cur->len == 0) {
        dst = cur;
    } else {
        int32_t idx = pad_new_doc();
        if (idx < 0) { pad_set_msg("Too many tabs", true); return; }
        dst = pad_cur();
    }
    if (!pad_load(dst, full)) { pad_set_msg("Open failed", true); return; }
    pad_sync_browser_to_doc(dst);
    if (dst->len == 0 && dst->path[0]) pad_set_msg("New file", false);
    else pad_set_msg("", false);
    pad_close_armed = false;
    pad_exit_armed = false;
}

static void pad_do_save() {
    PadDoc* t = pad_cur();
    if (!t) return;
    if (!t->path[0]) {
        pad_prompt = PAD_PR_SAVEAS;
        pad_prompt_buf[0] = 0;
        pad_set_msg("", false);
        return;
    }
    if (pad_save(t)) {
        pad_set_msg("Saved", false);
        pad_sync_browser_to_doc(t);
    } else {
        pad_set_msg("Save failed", true);
    }
    pad_close_armed = false;
    pad_exit_armed = false;
}

static void pad_do_find(bool next) {
    PadDoc* t = pad_cur();
    if (!t) return;
    const char* ndl = next ? pad_last_find : pad_prompt_buf;
    if (!ndl || !*ndl) {
        pad_prompt = PAD_PR_FIND;
        pad_prompt_buf[0] = 0;
        return;
    }
    if (!next) np_cpy(pad_last_find, ndl, sizeof(pad_last_find));
    PadGeo g = pad_geo(&pad_win);
    bool wrapped = false;
    if (pad_find_next(t, ndl, &wrapped)) {
        pad_show_cursor(t, g.rows, g.cols);
        pad_set_msg(wrapped ? "Found (wrapped)" : "Found", false);
    } else {
        pad_set_msg("Not found", true);
    }
}

static void pad_prompt_confirm() {
    if (pad_prompt == PAD_PR_OPEN) {
        char tmp[PAD_PROMPT_CAP];
        np_cpy(tmp, pad_prompt_buf, sizeof(tmp));
        pad_prompt = PAD_PR_NONE;
        pad_prompt_buf[0] = 0;
        pad_open_path(tmp);
    } else if (pad_prompt == PAD_PR_SAVEAS) {
        char tmp[PAD_PROMPT_CAP];
        np_cpy(tmp, pad_prompt_buf, sizeof(tmp));
        pad_prompt = PAD_PR_NONE;
        pad_prompt_buf[0] = 0;
        char full[PAD_PATH_CAP];
        pad_resolve(tmp, full, sizeof(full));
        if (!full[0]) { pad_set_msg("Bad path", true); return; }
        PadDoc* t = pad_cur();
        if (t && pad_save_as(t, full)) {
            pad_set_msg("Saved", false);
            pad_sync_browser_to_doc(t);
        } else {
            pad_set_msg("Save failed", true);
        }
        pad_close_armed = false;
        pad_exit_armed = false;
    } else if (pad_prompt == PAD_PR_FIND) {
        pad_prompt = PAD_PR_NONE;
        pad_do_find(false);
    } else {
        pad_prompt = PAD_PR_NONE;
    }
}

static void pad_prompt_key(int32_t k) {
    if (k == 27) {
        pad_prompt = PAD_PR_NONE;
        pad_prompt_buf[0] = 0;
        pad_set_msg("Cancelled", false);
        return;
    }
    if (k == '\r' || k == '\n') {
        pad_prompt_confirm();
        return;
    }
    uint32_t n = np_strlen(pad_prompt_buf);
    if (k == '\b' || k == 127) {
        if (n) pad_prompt_buf[n - 1] = 0;
        return;
    }
    if (k >= ' ' && k <= '~' && n + 1 < sizeof(pad_prompt_buf)) {
        pad_prompt_buf[n] = (char)k;
        pad_prompt_buf[n + 1] = 0;
    }
}

static void pad_next_tab() {
    int32_t n = pad_count();
    if (n < 2) return;
    int32_t pos = 0;
    for (int32_t i = 0; i < n; i++) {
        if (pad_nth(i) == pad_active) { pos = i; break; }
    }
    pad_active = pad_nth((pos + 1) % n);
    pad_close_armed = false;
    PadDoc* t = pad_cur();
    if (t) pad_sync_browser_to_doc(t);
}

static void pad_close_current() {
    PadDoc* t = pad_cur();
    if (!t) return;
    if (t->dirty && !pad_close_armed) {
        pad_close_armed = true;
        pad_set_msg("Modified: again closes tab", true);
        return;
    }
    pad_close_armed = false;
    pad_exit_armed = false;
    pad_close_doc(pad_active);
    PadDoc* n = pad_cur();
    if (n) pad_sync_browser_to_doc(n);
    pad_set_msg("", false);
}

bool pad_app_init(const char* first_path) {
    pad_quit = false;
    pad_close_armed = false;
    pad_last_find[0] = 0;
    if (!pad_docs_init()) return false;
    char pwd[PAD_PATH_CAP];
    if (np_env_get("PWD", pwd, sizeof(pwd)) < 0) np_cpy(pwd, "/", sizeof(pwd));
    pad_dir_norm(pad_dir, sizeof(pad_dir), pwd);
    pad_brow_refresh();
    if (first_path && first_path[0]) {
        char full[PAD_PATH_CAP];
        pad_resolve(first_path, full, sizeof(full));
        if (full[0]) {
            PadDoc* t = pad_cur();
            if (t && pad_load(t, full)) {
                pad_sync_browser_to_doc(t);
                if (t->len == 0) pad_set_msg("New file", false);
            }
        }
    }
    uint32_t w = 960;
    uint32_t h = 600;
    NpDisplay d = np_display();
    if (d.ok) {
        w = d.width > 96 ? d.width - 96 : d.width;
        h = d.height > 96 ? d.height - 96 : d.height;
        if (w > 1100) w = 1100;
        if (h > 700) h = 700;
        if (w < 640) w = 640;
        if (h < 440) h = 440;
    }
    return np_win_center(&pad_win, "PureC Notepad", w, h);
}

bool pad_app_draw() {
    return pad_draw(&pad_win);
}

bool pad_app_should_quit() {
    return pad_quit;
}

void pad_app_quit() {
    np_win_close(&pad_win);
}

static void pad_after_edit() {
    PadDoc* t = pad_cur();
    if (!t) return;
    PadGeo g = pad_geo(&pad_win);
    pad_show_cursor(t, g.rows, g.cols);
    pad_close_armed = false;
    pad_exit_armed = false;
    if (pad_msg_err) pad_set_msg("", false);
}

void pad_app_key(int32_t k) {
    if (pad_prompt != PAD_PR_NONE) {
        pad_prompt_key(k);
        return;
    }
    PadDoc* t = pad_cur();
    if (!t) return;
    if (k == 19) { pad_do_save(); return; }
    if (k == 15) {
        pad_prompt = PAD_PR_OPEN;
        pad_prompt_buf[0] = 0;
        pad_set_msg("", false);
        return;
    }
    if (k == 14) {
        if (pad_new_doc() < 0) pad_set_msg("Too many tabs", true);
        else pad_set_msg("", false);
        pad_close_armed = false;
        return;
    }
    if (k == 23) { pad_close_current(); return; }
    if (k == 6) {
        pad_prompt = PAD_PR_FIND;
        pad_prompt_buf[0] = 0;
        pad_set_msg("", false);
        return;
    }
    if (k == 7) { pad_do_find(true); return; }
    if (k == 20) { pad_next_tab(); return; }
    if (k == 24) {
        if (pad_any_dirty() && !pad_exit_armed) {
            pad_exit_armed = true;
            pad_set_msg("Modified: Ctrl+X again exits", true);
            return;
        }
        pad_quit = true;
        return;
    }
    if (k == '\b' || k == 127) {
        if (pad_back(t)) pad_after_edit();
        return;
    }
    if (k == '\r') { pad_newline(t); pad_after_edit(); return; }
    if (k == '\t') {
        if (pad_tab(t)) pad_after_edit();
        else pad_set_msg("No memory", true);
        return;
    }
    if (k != '\n' && (k < ' ' || k > '~')) return;
    char c = (char)k;
    if (c == '\n') { pad_newline(t); pad_after_edit(); return; }
    if (!pad_put(t, c)) { pad_set_msg("No memory", true); return; }
    pad_after_edit();
}

void pad_app_special(uint8_t k) {
    if (pad_prompt != PAD_PR_NONE) return;
    PadDoc* t = pad_cur();
    if (!t) return;
    PadGeo g = pad_geo(&pad_win);
    uint32_t page = g.rows > 1 ? g.rows - 1 : 1;
    bool moved = true;
    switch (k) {
        case NP_KEY_HOME: t->cursor = pad_lstart(t, t->cursor); break;
        case NP_KEY_END: t->cursor = pad_lend(t, t->cursor); break;
        case NP_KEY_PGUP: pad_vmove(t, -(int32_t)page); break;
        case NP_KEY_PGDN: pad_vmove(t, (int32_t)page); break;
        case NP_KEY_LEFT: if (t->cursor) t->cursor--; break;
        case NP_KEY_RIGHT: if (t->cursor < t->len) t->cursor++; break;
        case NP_KEY_UP: pad_vmove(t, -1); break;
        case NP_KEY_DOWN: pad_vmove(t, 1); break;
        case NP_KEY_DEL: if (pad_del(t)) { pad_after_edit(); return; } break;
        case NP_KEY_F1:
        case NP_KEY_F2: pad_do_find(true); return;
        case NP_KEY_F3: pad_do_find(true); return;
        default: moved = false; break;
    }
    if (moved) {
        pad_show_cursor(t, g.rows, g.cols);
        pad_close_armed = false;
    }
}

static void pad_click_to_cursor(PadDoc* t, PadGeo* g, uint32_t row, uint32_t c) {
    uint32_t vl = t->vline + row;
    uint32_t total = pad_lines(t);
    if (vl >= total) vl = total - 1;
    uint32_t lo = pad_loff(t, vl);
    uint32_t le = pad_lend(t, lo);
    uint32_t want = t->vcol + c;
    uint32_t acc = 0;
    uint32_t pos = lo;
    for (uint32_t i = lo; i < le; i++) {
        uint32_t adv = t->data[i] == '\t' ? 4 : 1;
        if (want <= acc + (adv > 2 ? 2 : adv)) { pos = i; break; }
        if (want <= acc + adv) { pos = i + 1; break; }
        acc += adv;
        pos = i + 1;
    }
    t->cursor = pos;
    pad_show_cursor(t, g->rows, g->cols);
    pad_close_armed = false;
}

static void pad_open_browser_entry(int32_t idx) {
    if (idx < 0 || idx >= pad_entry_count) return;
    pad_entry_sel = idx;
    if (pad_entries[idx].dir) {
        char np[PAD_PATH_CAP];
        if (pad_dir_join(np, sizeof(np), pad_dir, pad_entries[idx].name)) {
            pad_dir_norm(pad_dir, sizeof(pad_dir), np);
            pad_brow_refresh();
        }
        return;
    }
    PadDoc* t = pad_cur();
    if (t && t->dirty) {
        pad_set_msg("Modified: save first (Ctrl+S)", true);
        return;
    }
    char fp[PAD_PATH_CAP];
    if (!pad_dir_join(fp, sizeof(fp), pad_dir, pad_entries[idx].name)) return;
    PadDoc* dst = t;
    if (t && (t->dirty || t->len)) {
        int32_t ni = pad_new_doc();
        if (ni < 0) { pad_set_msg("Too many tabs", true); return; }
        dst = pad_cur();
    }
    if (dst && pad_load(dst, fp)) {
        pad_sync_browser_to_doc(dst);
        pad_set_msg("", false);
        pad_close_armed = false;
        pad_exit_armed = false;
    } else {
        pad_set_msg("Open failed", true);
    }
}

void pad_app_mouse(NpEvent* e) {
    if (!e || e->type != NP_EV_MOUSE_UP || e->button != 1) return;
    NpRect c = np_win_client(&pad_win);
    int32_t cx = e->x - (int32_t)c.x;
    int32_t cy = e->y - (int32_t)c.y;
    if (cx < 0 || cy < 0 || (uint32_t)cx >= c.w || (uint32_t)cy >= c.h) return;
    PadGeo g = pad_geo(&pad_win);
    uint32_t tb = pad_click_toolbar(&g, cx, cy);
    if (tb) {
        if (tb == 1) {
            if (pad_new_doc() < 0) pad_set_msg("Too many tabs", true);
            else pad_set_msg("", false);
        } else if (tb == 2) {
            pad_prompt = PAD_PR_OPEN;
            pad_prompt_buf[0] = 0;
            pad_set_msg("", false);
        } else if (tb == 3) {
            pad_do_save();
        } else if (tb == 4) {
            pad_prompt = PAD_PR_SAVEAS;
            pad_prompt_buf[0] = 0;
            pad_set_msg("", false);
        } else if (tb == 5) {
            pad_prompt = PAD_PR_FIND;
            pad_prompt_buf[0] = 0;
            pad_set_msg("", false);
        }
        pad_close_armed = false;
        return;
    }
    int32_t tab = pad_click_tab(&g, cx, cy);
    if (tab == -2) {
        if (pad_new_doc() < 0) pad_set_msg("Too many tabs", true);
        else pad_set_msg("", false);
        return;
    }
    if (tab >= 0) {
        int32_t idx = pad_nth(tab);
        if (idx >= 0) {
            pad_active = idx;
            pad_close_armed = false;
            PadDoc* t = pad_cur();
            if (t) pad_sync_browser_to_doc(t);
        }
        return;
    }
    int32_t be = pad_click_browser(&g, cx, cy);
    if (be == -2) {
        char up[PAD_PATH_CAP];
        pad_dir_up(up, sizeof(up), pad_dir);
        pad_dir_norm(pad_dir, sizeof(pad_dir), up);
        pad_brow_refresh();
        return;
    }
    if (be >= 0) {
        pad_open_browser_entry(be);
        return;
    }
    int dir = 0;
    if (pad_click_scroll(&g, cx, cy, &dir)) {
        PadDoc* t = pad_cur();
        if (t) {
            if (dir < 0) pad_vmove(t, -((int32_t)g.rows - 1));
            else if (dir > 0) pad_vmove(t, (int32_t)g.rows - 1);
            pad_show_cursor(t, g.rows, g.cols);
        }
        return;
    }
    uint32_t row = 0;
    uint32_t col = 0;
    if (pad_click_text(&g, cx, cy, &row, &col)) {
        PadDoc* t = pad_cur();
        if (t) pad_click_to_cursor(t, &g, row, col);
    }
}
