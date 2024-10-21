#pragma once

#include <Windows.h>

void hook_function(HMODULE mod, const char *name, void *detour, void **orig);
