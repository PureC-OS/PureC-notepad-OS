#include "gui/win.hpp"
#include "sys/np_os.hpp"
#include "com/str.hpp"

#define NP_BAR_H 26
#define NP_BD 2
#define NP_MIN_W 160
#define NP_MIN_H 96

#define NP_C_DESK 0x14141F
#define NP_C_WIN 0x23232E
#define NP_C_BAR 0x2E2E3C
#define NP_C_BD 0x4A4A5E
#define NP_C_TXT 0xD5D5E8
#define NP_C_MUT 0x8E8EA3
#define NP_C_ACC 0x7FA8F0
#define NP_C_DGR 0xE08AA0
#define NP_C_SHD 0x0E0E14

static void np_client_sync(NpWin* w) {
    w->client.x = w->frame.x + NP_BD;
    w->client.y = w->frame.y + NP_BAR_H + NP_BD;
    w->client.w = w->frame.w - NP_BD * 2;
    w->client.h = w->frame.h - NP_BAR_H - NP_BD * 2;
}

static bool np_push_frame(NpWin* w) {
    uint32_t h = w->min ? NP_BAR_H + NP_BD * 2 : w->frame.h;
    return np_win_update(w->frame.x, w->frame.y, w->frame.w, h);
}

bool np_point_in(int32_t x, int32_t y, NpRect r) {
    return x >= (int32_t)r.x && y >= (int32_t)r.y && x < (int32_t)(r.x + r.w) && y < (int32_t)(r.y + r.h);
}

static void np_clip_text(uint32_t x, uint32_t y, const char* s, uint32_t fg, uint32_t bg, NpRect clip) {
    if (!s || y < clip.y || y + 8 > clip.y + clip.h) return;
    char chunk[65];
    while (*s && x < clip.x + clip.w) {
        uint32_t n = 0;
        while (s[n] && n < 64 && x + (n + 1) * 8 <= clip.x + clip.w) { chunk[n] = s[n]; n++; }
        if (!n) return;
        chunk[n] = 0;
        np_text(x, y, chunk, fg, bg);
        x += n * 8;
        s += n;
    }
}

bool np_win_init(NpWin* w, const char* title, uint32_t x, uint32_t y, uint32_t ww, uint32_t hh) {
    if (!w || ww < NP_MIN_W || hh < NP_MIN_H) return false;
    NpDisplay d = np_display();
    if (!d.ok || ww > d.width || hh > d.height) return false;
    if (x > d.width - ww) x = d.width - ww;
    if (y > d.height - hh) y = d.height - hh;
    w->frame.x = x;
    w->frame.y = y;
    w->frame.w = ww;
    w->frame.h = hh;
    np_cpy(w->title, title ? title : "pad", sizeof(w->title));
    w->pmx = -1;
    w->pmy = -1;
    w->dox = 0;
    w->doy = 0;
    w->pbtn = 0;
    w->drag = false;
    w->min = false;
    w->open = true;
    w->focus = true;
    w->reg = false;
    w->repaint = false;
    np_client_sync(w);
    if (!np_win_register(x, y, ww, hh)) { w->open = false; return false; }
    w->reg = true;
    return true;
}

bool np_win_center(NpWin* w, const char* title, uint32_t ww, uint32_t hh) {
    NpDisplay d = np_display();
    if (!d.ok || ww > d.width || hh > d.height) return false;
    return np_win_init(w, title, (d.width - ww) / 2, (d.height - hh) / 2, ww, hh);
}

void np_win_begin(NpWin* w) {
    if (!w || !w->open) return;
    np_begin_frame();
    uint32_t hh = w->min ? NP_BAR_H + NP_BD * 2 : w->frame.h;
    np_rect(w->frame.x + 6, w->frame.y + 6, w->frame.w, hh, NP_C_SHD);
    np_rect(w->frame.x, w->frame.y, w->frame.w, hh, NP_C_BD);
    np_rect(w->frame.x + NP_BD, w->frame.y + NP_BD, w->frame.w - NP_BD * 2, NP_BAR_H, NP_C_BAR);
    if (!w->min) np_rect(w->client.x, w->client.y, w->client.w, w->client.h, NP_C_WIN);
    np_clip_text(w->frame.x + 10, w->frame.y + 10, w->title, NP_C_TXT, NP_C_BAR, w->frame);
    np_rect(w->frame.x + w->frame.w - 24, w->frame.y + 8, 14, 14, NP_C_DGR);
    np_text(w->frame.x + w->frame.w - 21, w->frame.y + 11, "x", NP_C_WIN, NP_C_DGR);
    np_rect(w->frame.x + w->frame.w - 44, w->frame.y + 8, 14, 14, NP_C_ACC);
    np_text(w->frame.x + w->frame.w - 41, w->frame.y + 11, w->min ? "+" : "-", NP_C_WIN, NP_C_ACC);
}

void np_win_end(NpWin* w) {
    if (!w || !w->open) return;
    np_end_frame();
    if (w->repaint) { w->repaint = false; np_win_repaint_done(); }
}

void np_win_close(NpWin* w) {
    if (!w) return;
    w->open = false;
    if (w->reg) { np_win_unregister(); w->reg = false; }
    np_redraw_desktop();
}

bool np_win_move(NpWin* w, uint32_t x, uint32_t y) {
    if (!w || !w->open) return false;
    NpDisplay d = np_display();
    if (!d.ok || w->frame.w > d.width || w->frame.h > d.height) return false;
    if (x > d.width - w->frame.w) x = d.width - w->frame.w;
    if (y > d.height - w->frame.h) y = d.height - w->frame.h;
    if (x == w->frame.x && y == w->frame.y) return true;
    w->frame.x = x;
    w->frame.y = y;
    np_client_sync(w);
    np_push_frame(w);
    np_redraw_desktop();
    return true;
}

bool np_win_opened(const NpWin* w) {
    return w && w->open;
}

bool np_win_minimized(const NpWin* w) {
    return w && w->open && w->min;
}

NpRect np_win_client(const NpWin* w) {
    if (!w) { NpRect z; z.x = 0; z.y = 0; z.w = 0; z.h = 0; return z; }
    return w->client;
}

void np_win_clear(NpWin* w, uint32_t color) {
    if (w && w->open && !w->min) np_rect(w->client.x, w->client.y, w->client.w, w->client.h, color);
}

void np_win_rect(NpWin* w, NpRect b, uint32_t color) {
    if (!w || !w->open || w->min || !b.w || !b.h) return;
    NpRect s;
    s.x = b.x + w->client.x;
    s.y = b.y + w->client.y;
    s.w = b.w;
    s.h = b.h;
    uint32_t rx = w->client.x + w->client.w;
    uint32_t by = w->client.y + w->client.h;
    if (s.x >= rx || s.y >= by) return;
    if (s.w > rx - s.x) s.w = rx - s.x;
    if (s.h > by - s.y) s.h = by - s.y;
    np_rect(s.x, s.y, s.w, s.h, color);
}

void np_win_text(NpWin* w, uint32_t x, uint32_t y, const char* s, uint32_t color) {
    if (!w || !w->open || w->min) return;
    np_clip_text(w->client.x + x, w->client.y + y, s, color, NP_C_WIN, w->client);
}

static uint8_t np_changed_btn(uint8_t c) {
    if (c & 1) return 1;
    if (c & 2) return 2;
    if (c & 4) return 3;
    return 0;
}

bool np_win_poll(NpWin* w, NpEvent* e) {
    if (!w || !e || !w->open) return false;
    e->type = NP_EV_NONE;
    e->x = 0;
    e->y = 0;
    e->key = 0;
    e->button = 0;
    uint32_t st = np_win_state();
    bool foc = (st & NP_WIN_FOCUSED) != 0;
    if (st & NP_WIN_REPAINT) {
        w->focus = foc;
        w->repaint = true;
        e->type = NP_EV_REPAINT;
        return true;
    }
    if (foc != w->focus) {
        w->focus = foc;
        if (foc) { e->type = NP_EV_FOCUS; return true; }
    }
    if (foc) {
        int32_t k = np_try_char();
        if (k >= 0) {
            e->key = k;
            if (k == 27) { e->type = NP_EV_CLOSE; np_win_close(w); }
            else e->type = NP_EV_KEY;
            return true;
        }
        int32_t sp = np_try_special();
        if (sp >= 0) { e->key = sp; e->type = NP_EV_SPECIAL; return true; }
    }
    NpMouse m;
    m.x = 0;
    m.y = 0;
    m.buttons = 0;
    if (!np_mouse(&m)) return false;
    e->x = m.x;
    e->y = m.y;
    uint8_t ch = (uint8_t)(m.buttons ^ w->pbtn);
    if (ch) {
        e->button = np_changed_btn(ch);
        e->type = (m.buttons & ch) ? NP_EV_MOUSE_DOWN : NP_EV_MOUSE_UP;
    } else if (m.x != w->pmx || m.y != w->pmy) {
        e->type = NP_EV_MOUSE_MOVE;
    }
    w->pmx = m.x;
    w->pmy = m.y;
    w->pbtn = m.buttons;
    if (!foc) return false;
    NpRect close;
    close.x = w->frame.x + w->frame.w - 24;
    close.y = w->frame.y + 8;
    close.w = 14;
    close.h = 14;
    NpRect mini;
    mini.x = w->frame.x + w->frame.w - 44;
    mini.y = w->frame.y + 8;
    mini.w = 14;
    mini.h = 14;
    if (e->type == NP_EV_MOUSE_DOWN && e->button == 1) {
        NpRect bar;
        bar.x = w->frame.x;
        bar.y = w->frame.y;
        bar.w = w->frame.w;
        bar.h = NP_BAR_H + NP_BD * 2;
        if (np_point_in(m.x, m.y, bar) && !np_point_in(m.x, m.y, close) && !np_point_in(m.x, m.y, mini)) {
            w->drag = true;
            w->dox = m.x - (int32_t)w->frame.x;
            w->doy = m.y - (int32_t)w->frame.y;
        }
    }
    if (e->type == NP_EV_MOUSE_MOVE && w->drag && (m.buttons & 1)) {
        int32_t nx = m.x - w->dox;
        int32_t ny = m.y - w->doy;
        if (nx < 0) nx = 0;
        if (ny < 0) ny = 0;
        if (np_win_move(w, (uint32_t)nx, (uint32_t)ny)) e->type = NP_EV_MOVE;
    }
    if (e->type == NP_EV_MOUSE_UP && e->button == 1) {
        bool was = w->drag;
        w->drag = false;
        if (!was && np_point_in(m.x, m.y, close)) {
            e->type = NP_EV_CLOSE;
            np_win_close(w);
            return true;
        }
        if (!was && np_point_in(m.x, m.y, mini)) {
            w->min = !w->min;
            np_push_frame(w);
            np_redraw_desktop();
            e->type = NP_EV_MIN;
        }
    }
    if (!(m.buttons & 1)) w->drag = false;
    return e->type != NP_EV_NONE;
}
