#pragma once
#include <stdint.h>
bool pad_load(PadDoc* t, const char* path);
bool pad_save(PadDoc* t);
bool pad_save_as(PadDoc* t, const char* path);
void pad_resolve(const char* arg, char* out, uint32_t cap);