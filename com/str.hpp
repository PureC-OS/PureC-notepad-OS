#pragma once
#include <stdint.h>

uint32_t np_strlen(const char* s);
int np_cmp(const char* a, const char* b);
void np_cpy(char* dst, const char* src, uint32_t cap);
void np_u32dec(char* out, uint32_t v);
void np_u32hex(char* out, uint32_t v, uint32_t digits);
void np_append(char* dst, const char* src, uint32_t cap);
