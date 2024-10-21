#include <Windows.h>

#include <stdio.h>

#include <MinHook.h>

#include "net.h"

static BOOL gAttached = FALSE;

static void hook_attach() {
    if (gAttached == TRUE) {
        return;
    }

#if _DEBUG
    AllocConsole();
    FILE *fDummy = NULL;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
#endif

    if (MH_Initialize() != MH_OK) {
#if _DEBUG
        fprintf(stderr, "Failed to initialize MinHook\n");
        FreeConsole();
#endif
        return;
    }

    net_init();

    MH_EnableHook(MH_ALL_HOOKS);

    gAttached = TRUE;
}

static void hook_detatch() {
    MH_Uninitialize();

#if _DEBUG
    FreeConsole();
#endif

    gAttached = FALSE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            hook_attach();
            break;

        case DLL_PROCESS_DETACH:
            hook_detatch();
            break;
    }
    return TRUE;
}
