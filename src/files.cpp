#include "pad/files.hpp"
#include "pad/docs.hpp"
#include "com/str.hpp"
#include "sys/np_os.hpp"

void pad_resolve(const char* arg, char* out, uint32_t cap) {
    if (!arg || !*arg || !out || !cap) return;
    while (*arg == ' ' || *arg == '\t') arg++;
    uint32_t n = 0;
    while (arg[n] && arg[n] != ' ' && arg[n] != '\t') n++;
    if (!n) { out[0] = 0; return; }
    if (arg[0] != '/') {
        char pwd[PAD_PATH_CAP];
        if (np_env_get("PWD", pwd, sizeof(pwd)) < 0) np_cpy(pwd, "/", sizeof(pwd));
        uint32_t dl = np_strlen(pwd);
        uint32_t need = dl + (dl > 1 ? 1 : 0) + n + 1;
        if (need > cap) { out[0] = 0; return; }
        np_cpy(out, pwd, cap);
        if (dl > 1) { out[dl] = '/'; out[dl + 1] = 0; dl++; }
        for (uint32_t i = 0; i < n; i++) out[dl + i] = arg[i];
        out[dl + n] = 0;
    } else {
        if (n + 1 > cap) { out[0] = 0; return; }
        for (uint32_t i = 0; i < n; i++) out[i] = arg[i];
        out[n] = 0;
    }
}

bool pad_load(PadDoc* t, const char* path) {
    if (!t || !path || !*path) return false;
    t->len = 0;
    t->cursor = 0;
    t->vline = 0;
    t->vcol = 0;
    t->dirty = false;
    int32_t fd = np_open(path);
    if (fd < 0) {
        np_cpy(t->path, path, sizeof(t->path));
        return true;
    }
    for (;;) {
        char chunk[256];
        int32_t c = np_read(fd, chunk, sizeof(chunk));
        if (c < 0) { np_close(fd); return false; }
        if (!c) break;
        if (t->len + (uint32_t)c > PAD_TAB_CAP) { np_close(fd); t->len = 0; return false; }
        for (int32_t i = 0; i < c; i++) t->data[t->len++] = chunk[i];
    }
    np_close(fd);
    np_cpy(t->path, path, sizeof(t->path));
    return true;
}

bool pad_save(PadDoc* t) {
    if (!t || !t->path[0]) return false;
    if (np_write_file(t->path, t->data, t->len) < 0) {
        if (np_create_file(t->path) < 0) return false;
        if (np_write_file(t->path, t->data, t->len) < 0) return false;
    }
    t->dirty = false;
    return true;
}

bool pad_save_as(PadDoc* t, const char* path) {
    if (!t || !path || !*path) return false;
    np_cpy(t->path, path, sizeof(t->path));
    return pad_save(t);
}
