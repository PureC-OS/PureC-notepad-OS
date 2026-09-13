#include "pad/view.hpp"
#include "pad/docs.hpp"
#include "pad/browser.hpp"
#include "pad/cfg.hpp"
#include "com/str.hpp"

PadGeo pad_geo(NpWin* w) {
    PadGeo g;
    g.cw = 0;
    g.ch = 0;
    g.rows = 1;
    g.cols = 8;
    g.tx = 0;
    g.ty = 0;
    g.tw = 0;
    g.th = 0;
    g.tab_w = 0;
    g.tab_n = 0;
    g.brow_ly = 0;
    g.sb_x = 0;
    g.sb_y = 0;
    g.sb_h = 0;
    g.th_y = 0;
    g.th_h = 0;
    if (!w) return g;
    NpRect c = np_win_client(w);
    g.cw = c.w;
    g.ch = c.h;
    g.rows = pad_vis_rows(c.h);
    g.cols = pad_vis_cols(c.w);
    g.tx = PAD_BROW_W + PAD_GUT_W;
    g.ty = PAD_TOOL_H + PAD_TABS_H;
    g.tw = c.w > g.tx ? c.w - g.tx : 0;
    g.th = g.rows * PAD_ROW_H;
    int32_t n = pad_count();
    if (n < 1) n = 1;
    g.tab_n = (uint32_t)n;
    uint32_t avail = c.w > 60 ? c.w - 60 : 40;
    uint32_t tw = avail / (uint32_t)n;
    if (tw > 150) tw = 150;
    if (tw < 24) tw = 24;
    g.tab_w = tw;
    g.brow_ly = PAD_TOOL_H + PAD_TABS_H + 46;
    g.sb_x = c.w > g.tx + 8 ? c.w - 8 : g.tx;
    g.sb_y = g.ty;
    g.sb_h = g.th;
    PadDoc* t = pad_cur();
    uint32_t total = t ? pad_lines(t) : 1;
    uint32_t hh = g.th > 12 ? (g.rows * g.th) / (total > g.rows ? total : g.rows) : g.th;
    if (hh < 12) hh = 12;
    if (hh > g.th) hh = g.th;
    uint32_t maxl = total > g.rows ? total - g.rows : 0;
    uint32_t vl = t ? t->vline : 0;
    if (vl > maxl) vl = maxl;
    g.th_h = hh;
    g.th_y = maxl ? g.sb_y + (vl * (g.sb_h - hh)) / maxl : g.sb_y;
    return g;
}

uint32_t pad_click_toolbar(PadGeo* g, int32_t cx, int32_t cy) {
    if (!g || cy < 5 || cy >= 25) return 0;
    for (uint32_t i = 0; i < 5; i++) {
        int32_t x = 8 + (int32_t)(i * 70);
        if (cx >= x && cx < x + 64) return i + 1;
    }
    return 0;
}

int32_t pad_click_tab(PadGeo* g, int32_t cx, int32_t cy) {
    if (!g) return -1;
    if (cy >= (int32_t)PAD_TOOL_H && cy < (int32_t)(PAD_TOOL_H + PAD_TABS_H)) {
        if (cx >= (int32_t)(g->cw - 32)) return -2;
        int32_t n = pad_count();
        for (int32_t i = 0; i < n; i++) {
            int32_t x = 8 + i * (int32_t)g->tab_w;
            if (cx >= x && cx < x + (int32_t)g->tab_w - 4) return i;
        }
    }
    return -1;
}

int32_t pad_click_browser(PadGeo* g, int32_t cx, int32_t cy) {
    if (!g || cx < 0 || cx >= (int32_t)PAD_BROW_W) return -1;
    uint32_t top = PAD_TOOL_H + PAD_TABS_H;
    if (cy >= (int32_t)(top + 24) && cy < (int32_t)(top + 42)) return -2;
    if (cy < (int32_t)g->brow_ly) return -1;
    uint32_t row = ((uint32_t)cy - g->brow_ly) / 18;
    uint32_t idx = (uint32_t)pad_entry_top + row;
    if ((int32_t)idx >= pad_entry_count) return -1;
    return (int32_t)idx;
}

bool pad_click_text(PadGeo* g, int32_t cx, int32_t cy, uint32_t* line, uint32_t* col) {
    if (!g || !line || !col) return false;
    if (cx < (int32_t)g->tx || cy < (int32_t)g->ty) return false;
    if (cx >= (int32_t)(g->tx + g->tw - 8) || cy >= (int32_t)(g->ty + g->th)) return false;
    *line = ((uint32_t)cy - g->ty) / PAD_ROW_H;
    *col = ((uint32_t)cx - g->tx) / PAD_COL_W;
    return true;
}

bool pad_click_scroll(PadGeo* g, int32_t cx, int32_t cy, int* dir) {
    if (!g || !dir) return false;
    if (cx < (int32_t)g->sb_x || cx >= (int32_t)(g->sb_x + 6)) return false;
    if (cy < (int32_t)g->sb_y || cy >= (int32_t)(g->sb_y + g->sb_h)) return false;
    if (cy < (int32_t)g->th_y) *dir = -1;
    else if (cy >= (int32_t)(g->th_y + g->th_h)) *dir = 1;
    else *dir = 0;
    return true;
}

static uint32_t pad_exp_col(PadDoc* t, uint32_t off) {
    uint32_t s = pad_lstart(t, off);
    uint32_t c = 0;
    for (uint32_t i = s; i < off && i < t->len; i++) {
        if (t->data[i] == '\t') c += 4;
        else c++;
    }
    return c;
}

static void pad_base_name(const char* p, char* out, uint32_t cap) {
    if (!p || !*p) { np_cpy(out, "untitled", cap); return; }
    const char* b = p;
    for (const char* q = p; *q; q++) if (*q == '/') b = q + 1;
    if (!*b) { np_cpy(out, "/", cap); return; }
    np_cpy(out, b, cap);
}

static void pad_draw_toolbar(NpWin* w, PadGeo* g) {
    NpRect r;
    r.x = 0; r.y = 0; r.w = g->cw; r.h = PAD_TOOL_H;
    np_win_rect(w, r, PAD_PANEL);
    const char* labels[5];
    labels[0] = "New";
    labels[1] = "Open";
    labels[2] = "Save";
    labels[3] = "SaveAs";
    labels[4] = "Find";
    for (uint32_t i = 0; i < 5; i++) {
        NpRect b;
        b.x = 8 + i * 70; b.y = 5; b.w = 64; b.h = 20;
        np_win_rect(w, b, PAD_LINE);
        np_win_text(w, b.x + 34 - np_strlen(labels[i]) * 4, 10, labels[i], 0xD5D5E8);
    }
    PadDoc* t = pad_cur();
    char info[96];
    info[0] = 0;
    if (t) {
        np_cpy(info, t->path[0] ? t->path : "(untitled)", sizeof(info));
        char dec[12];
        np_u32dec(dec, t->len);
        np_append(info, "  ", sizeof(info));
        np_append(info, dec, sizeof(info));
        np_append(info, " B", sizeof(info));
        if (t->dirty) np_append(info, " *", sizeof(info));
    }
    uint32_t iw = np_strlen(info) * 8;
    if (iw + 400 < g->cw) np_win_text(w, g->cw - iw - 8, 10, info, t && t->dirty ? PAD_YLW : PAD_GRY);
}

static void pad_draw_tabs(NpWin* w, PadGeo* g) {
    NpRect r;
    r.x = 0; r.y = PAD_TOOL_H; r.w = g->cw; r.h = PAD_TABS_H;
    np_win_rect(w, r, PAD_PANEL2);
    int32_t n = pad_count();
    int32_t seen = 0;
    for (uint32_t j = 0; j < PAD_MAX_TABS; j++) {
        if (!pad_docs[j].used) continue;
        int32_t i = seen++;
        if (i >= n) break;
        NpRect b;
        b.x = 8 + (uint32_t)i * g->tab_w;
        b.y = PAD_TOOL_H + 2;
        b.w = g->tab_w > 4 ? g->tab_w - 4 : g->tab_w;
        b.h = PAD_TABS_H - 4;
        bool act = (int32_t)idx == pad_active;
        np_win_rect(w, b, act ? PAD_CUR : PAD_PANEL);
        char nm[40];
        pad_base_name(pad_docs[idx].path, nm, sizeof(nm));
        if (pad_docs[idx].dirty) np_append(nm, "*", sizeof(nm));
        char vis[24];
        uint32_t maxc = b.w > 12 ? (b.w - 12) / 8 : 4;
        if (maxc > sizeof(vis) - 1) maxc = sizeof(vis) - 1;
        for (uint32_t c = 0; c < maxc && nm[c]; c++) vis[c] = nm[c];
        vis[maxc] = 0;
        uint32_t nl = np_strlen(vis);
        for (uint32_t c = nl; c < maxc; c++) vis[c] = 0;
        np_win_text(w, b.x + 6, PAD_TOOL_H + 7, vis, act ? 0xD5D5E8 : PAD_GRY);
    }
    NpRect pb;
    pb.x = g->cw > 32 ? g->cw - 32 : 0;
    pb.y = PAD_TOOL_H + 1;
    pb.w = 28;
    pb.h = PAD_TABS_H - 2;
    np_win_rect(w, pb, PAD_LINE);
    np_win_text(w, pb.x + 10, PAD_TOOL_H + 7, "+", PAD_GRN);
}

static void pad_draw_browser(NpWin* w, PadGeo* g) {
    uint32_t top = PAD_TOOL_H + PAD_TABS_H;
    uint32_t h = g->ch > top + PAD_STAT_H ? g->ch - top - PAD_STAT_H : 0;
    NpRect r;
    r.x = 0; r.y = top; r.w = PAD_BROW_W; r.h = h;
    np_win_rect(w, r, PAD_PANEL2);
    char dir[64];
    uint32_t dl = np_strlen(pad_dir);
    if (dl * 8 > PAD_BROW_W - 16) {
        uint32_t keep = (PAD_BROW_W - 16) / 8;
        dir[0] = '.';
        dir[1] = '.';
        for (uint32_t i = 0; i < keep - 2; i++) dir[2 + i] = pad_dir[dl - keep + 2 + i];
        dir[keep] = 0;
    } else {
        np_cpy(dir, pad_dir, sizeof(dir));
    }
    np_win_text(w, 8, top + 6, dir, 0xD5D5E8);
    NpRect sep;
    sep.x = 8; sep.y = top + 20; sep.w = PAD_BROW_W - 16; sep.h = 1;
    np_win_rect(w, sep, PAD_LINE);
    NpRect up;
    up.x = 8; up.y = top + 24; up.w = PAD_BROW_W - 16; up.h = 18;
    np_win_rect(w, up, PAD_LINE);
    np_win_text(w, 12, top + 28, ".. (up)", PAD_ACC);
    uint32_t rows = h > 50 ? (h - 50) / 18 : 1;
    pad_brow_rows = rows;
    if (pad_entry_top + (int32_t)rows > pad_entry_count && pad_entry_count > 0) {
        pad_entry_top = pad_entry_count - (int32_t)rows;
        if (pad_entry_top < 0) pad_entry_top = 0;
    }
    for (uint32_t i = 0; i < rows; i++) {
        int32_t idx = pad_entry_top + (int32_t)i;
        if (idx >= pad_entry_count) break;
        uint32_t y = g->brow_ly + i * 18;
        bool sel = idx == pad_entry_sel;
        NpRect b;
        b.x = 6; b.y = y; b.w = PAD_BROW_W - 12; b.h = 16;
        np_win_rect(w, b, sel ? PAD_CUR : (i % 2 ? PAD_PANEL : PAD_PANEL2));
        if (pad_entries[idx].dir) {
            NpRect f1;
            f1.x = 10; f1.y = y + 3; f1.w = 12; f1.h = 8;
            np_win_rect(w, f1, PAD_YLW);
            NpRect f2;
            f2.x = 10; f2.y = y + 8; f2.w = 14; f2.h = 6;
            np_win_rect(w, f2, PAD_YLW);
        } else {
            NpRect f;
            f.x = 10; f.y = y + 2; f.w = 10; f.h = 12;
            np_win_rect(w, f, 0xD5D5E8);
        }
        char nm[22];
        for (uint32_t c = 0; c < 20; c++) {
            nm[c] = pad_entries[idx].name[c];
            if (!nm[c]) break;
        }
        nm[20] = 0;
        np_win_text(w, 28, y + 4, nm, sel ? 0xD5D5E8 : (pad_entries[idx].dir ? PAD_ACC : 0xD5D5E8));
    }
}

static void pad_draw_text_area(NpWin* w, PadGeo* g) {
    PadDoc* t = pad_cur();
    NpRect r;
    r.x = PAD_BROW_W; r.y = g->ty; r.w = g->tw; r.h = g->th;
    np_win_rect(w, r, PAD_BG);
    NpRect sb;
    sb.x = g->sb_x - PAD_BROW_W - PAD_GUT_W; sb.y = 0; sb.w = 6; sb.h = g->sb_h;
    if (sb.x < g->tw) {
        NpRect full;
        full.x = PAD_BROW_W + g->tw - 8;
        full.y = g->ty;
        full.w = 6;
        full.h = g->sb_h;
        np_win_rect(w, full, PAD_PANEL);
        NpRect th;
        th.x = PAD_BROW_W + g->tw - 8;
        th.y = g->ty + (g->th_y - g->sb_y);
        th.w = 6;
        th.h = g->th_h;
        np_win_rect(w, th, PAD_GRY);
    }
    if (!t) return;
    uint32_t cur_line = pad_lnum(t, t->cursor);
    for (uint32_t row = 0; row < g->rows; row++) {
        uint32_t vl = t->vline + row;
        uint32_t y = g->ty + row * PAD_ROW_H;
        if (vl >= pad_lines(t)) break;
        if (vl == cur_line) {
            NpRect hl;
            hl.x = PAD_BROW_W + PAD_GUT_W;
            hl.y = y;
            hl.w = g->tw > 8 ? g->tw - 8 : 0;
            hl.h = PAD_ROW_H;
            np_win_rect(w, hl, PAD_CUR);
        }
        char nb[12];
        np_u32dec(nb, vl + 1);
        uint32_t nl = np_strlen(nb);
        np_win_text(w, PAD_BROW_W + PAD_GUT_W - 8 - nl * 8, y + 4, nb, vl == cur_line ? PAD_ACC : PAD_DGRY);
        uint32_t lo = pad_loff(t, vl);
        uint32_t le = pad_lend(t, lo);
        char out[256];
        uint32_t col = 0;
        uint32_t oi = 0;
        for (uint32_t i = lo; i < le && oi + 1 < sizeof(out); i++) {
            char c = t->data[i];
            if (c == '\t') {
                for (int k = 0; k < 4; k++) {
                    if (col >= t->vcol + g->cols) break;
                    if (col >= t->vcol) { out[oi++] = ' '; }
                    col++;
                }
            } else {
                if (col >= t->vcol + g->cols) break;
                if (col >= t->vcol) out[oi++] = (c < ' ' ? '.' : c);
                col++;
            }
        }
        out[oi] = 0;
        np_win_text(w, g->tx, y + 4, out, 0xD5D5E8);
    }
    uint32_t cc = pad_exp_col(t, t->cursor);
    if (cur_line >= t->vline && cur_line < t->vline + g->rows && cc >= t->vcol && cc < t->vcol + g->cols) {
        uint32_t sx = g->tx + (cc - t->vcol) * PAD_COL_W;
        uint32_t sy = g->ty + (cur_line - t->vline) * PAD_ROW_H + 2;
        NpRect cb;
        cb.x = sx;
        cb.y = sy;
        cb.w = 8;
        cb.h = 12;
        np_win_rect(w, cb, PAD_ACC);
        if (t->cursor < t->len && t->data[t->cursor] != '\n') {
            char s[2];
            s[0] = t->data[t->cursor] < ' ' ? '.' : t->data[t->cursor];
            s[1] = 0;
            np_win_text(w, sx, sy + 2, s, PAD_BG);
        }
    }
}

static void pad_draw_status(NpWin* w, PadGeo* g) {
    uint32_t y = g->ch > PAD_STAT_H ? g->ch - PAD_STAT_H : 0;
    NpRect r;
    r.x = 0; r.y = y; r.w = g->cw; r.h = PAD_STAT_H;
    np_win_rect(w, r, PAD_PANEL);
    if (pad_prompt != PAD_PR_NONE) {
        const char* lab = "";
        if (pad_prompt == PAD_PR_OPEN) lab = "Open: ";
        else if (pad_prompt == PAD_PR_SAVEAS) lab = "Save as: ";
        else if (pad_prompt == PAD_PR_FIND) lab = "Find: ";
        np_win_text(w, 8, y + 8, lab, 0xD5D5E8);
        uint32_t lx = 8 + np_strlen(lab) * 8 + 4;
        np_win_text(w, lx, y + 8, pad_prompt_buf, PAD_GRN);
        uint32_t cx = lx + np_strlen(pad_prompt_buf) * 8;
        NpRect cb;
        cb.x = cx; cb.y = y + 6; cb.w = 8; cb.h = 14;
        np_win_rect(w, cb, PAD_ACC);
        np_win_text(w, g->cw > 170 ? g->cw - 162 : 0, y + 8, "Enter ok Esc cancel", PAD_DGRY);
        return;
    }
    PadDoc* t = pad_cur();
    if (pad_msg[0]) {
        np_win_text(w, 8, y + 8, pad_msg, pad_msg_err ? PAD_RED : PAD_YLW);
    } else if (t) {
        char left[64];
        left[0] = 0;
        np_append(left, "Ln ", sizeof(left));
        char d[12];
        np_u32dec(d, pad_lnum(t, t->cursor) + 1);
        np_append(left, d, sizeof(left));
        np_append(left, " Col ", sizeof(left));
        np_u32dec(d, t->cursor - pad_lstart(t, t->cursor) + 1);
        np_append(left, d, sizeof(left));
        np_win_text(w, 8, y + 8, left, PAD_GRY);
    }
    const char* help = "Ctrl+S save Ctrl+O open Ctrl+N tab Ctrl+F find Ctrl+X exit";
    uint32_t hw = np_strlen(help) * 8;
    if (!pad_msg[0] && hw + 20 < g->cw) np_win_text(w, g->cw - hw - 8, y + 8, help, PAD_DGRY);
}

bool pad_draw(NpWin* w) {
    if (np_win_minimized(w)) return true;
    np_win_begin(w);
    if (np_win_minimized(w)) { np_win_end(w); return true; }
    PadGeo g = pad_geo(w);
    np_win_clear(w, PAD_BG);
    pad_draw_toolbar(w, &g);
    pad_draw_tabs(w, &g);
    NpRect sep;
    sep.x = PAD_BROW_W; sep.y = g.ty; sep.w = 1; sep.h = g.th;
    np_win_rect(w, sep, PAD_LINE);
    pad_draw_browser(w, &g);
    pad_draw_text_area(w, &g);
    pad_draw_status(w, &g);
    np_win_end(w);
    return true;
}
