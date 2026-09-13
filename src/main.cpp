#include "pad/input.hpp"
#include "pad/docs.hpp"
#include "pad/files.hpp"
#include "pad/view.hpp"
#include "com/str.hpp"
#include "sys/np_os.hpp"
#include "gui/win.hpp"

extern "C" void _start(void) {
    char args[PAD_PATH_CAP];
    int32_t n = np_cmdline(args, sizeof(args));
    char first[PAD_PATH_CAP];
    first[0] = 0;
    if (n > 0 && args[0]) {
        char* p = args;
        while (*p == ' ' || *p == '\t') p++;
        if (*p) {
            uint32_t len = 0;
            while (p[len] && p[len] != ' ' && p[len] != '\t') len++;
            char* rem = p + len;
            while (*rem == ' ' || *rem == '\t') rem++;
            if (!*rem && len + 1 < sizeof(first)) {
                for (uint32_t i = 0; i < len; i++) first[i] = p[i];
                first[len] = 0;
            }
        }
    }
    if (!pad_app_init(first[0] ? first : 0)) np_exit(1);
    pad_app_draw();
    for (;;) {
        NpEvent e;
        if (!np_win_poll(&pad_win, &e)) {
            np_sleep(16);
            continue;
        }
        if (e.type == NP_EV_CLOSE) {
            if (pad_any_dirty() && !pad_exit_armed) {
                pad_exit_armed = true;
                pad_set_msg("Modified: again exits", true);
                pad_app_draw();
                continue;
            }
            break;
        }
        if (e.type == NP_EV_MOVE || e.type == NP_EV_MIN || e.type == NP_EV_FOCUS || e.type == NP_EV_REPAINT) {
            pad_app_draw();
            continue;
        }
        if (e.type == NP_EV_MOUSE_DOWN || e.type == NP_EV_MOUSE_UP || e.type == NP_EV_MOUSE_MOVE) {
            if (e.type == NP_EV_MOUSE_UP) {
                pad_app_mouse(&e);
                pad_app_draw();
            }
            continue;
        }
        if (e.type == NP_EV_SPECIAL) {
            pad_app_special((uint8_t)e.key);
            pad_app_draw();
            continue;
        }
        if (e.type != NP_EV_KEY || np_win_minimized(&pad_win)) continue;
        pad_app_key(e.key);
        if (pad_app_should_quit()) break;
        pad_app_draw();
    }
    pad_app_quit();
    np_exit(0);
}
