#include "com/str.hpp"

extern "C" void* memcpy(void* d, const void* s, unsigned long n) {
    char* a = (char*)d;
    const char* b = (const char*)s;
    for (unsigned long i = 0; i < n; i++) a[i] = b[i];
    return d;
}

extern "C" void* memmove(void* d, const void* s, unsigned long n) {
    char* a = (char*)d;
    const char* b = (const char*)s;
    if (a < b) {
        for (unsigned long i = 0; i < n; i++) a[i] = b[i];
    } else if (a > b) {
        for (unsigned long i = n; i > 0; i--) a[i - 1] = b[i - 1];
    }
    return d;
}

extern "C" void* memset(void* d, int c, unsigned long n) {
    char* a = (char*)d;
    for (unsigned long i = 0; i < n; i++) a[i] = (char)c;
    return d;
}

uint32_t np_strlen(const char* s) {
    uint32_t n = 0;
    if (!s) return 0;
    while (s[n]) n++;
    return n;
}

int np_cmp(const char* a, const char* b) {
    if (!a) return b && *b ? -1 : 0;
    if (!b) return *a ? 1 : 0;
    while (*a && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

void np_cpy(char* dst, const char* src, uint32_t cap) {
    if (!cap || !dst) return;
    if (!src) { dst[0] = 0; return; }
    uint32_t i = 0;
    while (src[i] && i + 1 < cap) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

void np_u32dec(char* out, uint32_t v) {
    char rev[11];
    uint32_t n = 0;
    if (v == 0) rev[n++] = '0';
    else while (v) { rev[n++] = (char)('0' + v % 10); v /= 10; }
    uint32_t i = 0;
    while (n) out[i++] = rev[--n];
    out[i] = 0;
}

void np_u32hex(char* out, uint32_t v, uint32_t digits) {
    const char* h = "0123456789ABCDEF";
    for (uint32_t i = 0; i < digits; i++) out[digits - 1 - i] = h[(v >> (i * 4)) & 15];
    out[digits] = 0;
}

void np_append(char* dst, const char* src, uint32_t cap) {
    uint32_t p = np_strlen(dst);
    if (p >= cap) return;
    np_cpy(dst + p, src, cap - p);
}
