#include "pad/docs.hpp"
#include "com/str.hpp"
#include <cstdio>

static int fails = 0;

static void check(bool ok, const char* name) {
    if (!ok) {
        fails++;
        std::puts(name);
    }
}

int main() {
    check(pad_docs_init(), "init");
    PadDoc* t = pad_cur();
    check(t != 0, "cur");
    const char* text = "line1\n  indented\nline3";
    for (const char* p = text; *p; p++) {
        t->cursor = t->len;
        check(pad_put(t, *p), "put");
    }
    check(pad_lines(t) == 3, "lines");
    check(pad_lnum(t, 0) == 0, "lnum0");
    check(pad_loff(t, 1) == 6, "loff1");
    check(pad_lstart(t, 8) == 6, "lstart");
    check(pad_lend(t, 6) == 16, "lend");
    t->cursor = 6;
    check(pad_newline(t), "newline");
    check(t->data[6] == '\n', "nl char");
    check(t->data[7] == ' ' && t->data[8] == ' ', "indent kept");
    t->cursor = t->len;
    check(pad_tab(t), "tab");
    check(t->len >= 4, "tab len");
    uint32_t before = t->len;
    t->cursor = t->len;
    check(pad_back(t), "back");
    check(t->len + 1 == before, "back len");
    t->cursor = 0;
    check(pad_del(t), "del");
    bool wrapped = false;
    t->cursor = 0;
    check(pad_find_next(t, "line3", &wrapped), "find");
    check(!wrapped, "no wrap");
    t->cursor = t->len > 0 ? t->len - 1 : 0;
    check(pad_find_next(t, "line", &wrapped), "find wrap");
    check(wrapped, "wrapped");
    check(!pad_find_next(t, "zzz-not-present", &wrapped), "not found");
    check(pad_new_doc() >= 0, "second tab");
    check(pad_count() == 2, "count");
    check(pad_nth(0) != pad_nth(1), "nth");
    check(pad_close_doc(pad_active), "close");
    check(pad_count() == 1, "count after close");
    pad_vmove(t, 1000);
    check(pad_lnum(t, t->cursor) == pad_lines(t) - 1, "vmove clamp");
    pad_vmove(t, -1000);
    check(t->cursor == 0, "vmove top");
    PadDoc* c = pad_cur();
    c->cursor = c->len;
    for (int i = 0; i < 10; i++) pad_put(c, 'x');
    pad_show_cursor(c, 24, 80);
    check(c->vline == 0, "visible");
    if (fails == 0) std::puts("host test ok");
    return fails ? 1 : 0;
}
