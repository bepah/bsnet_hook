#include <stdio.h>

#include <MinHook.h>

#include "utils.h"

void hook_function(HMODULE mod, const char *name, void *detour, void **orig) {
	FARPROC proc = NULL;

	proc = GetProcAddress(mod, name);

	if (proc == NULL) {
#if _DEBUG
		fprintf(stderr, "[!] %s is NULL\n", name);
#endif
		return;
	}

	if (MH_CreateHook((void *) proc, detour, orig) != MH_OK) {
#if _DEBUG
		fprintf(stderr, "[!] Failed to hook %s\n", name);
#endif
	}
}
