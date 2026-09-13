#pragma once
#include <stdint.h>
#include "gui/win.hpp"

extern NpWin pad_win;

bool pad_app_init(const char* first_path);
bool pad_app_draw();
void pad_app_key(int32_t k);
void pad_app_special(uint8_t k);
void pad_app_mouse(NpEvent* e);
bool pad_app_should_quit();
void pad_app_quit();
