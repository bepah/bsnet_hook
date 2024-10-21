#include <Windows.h>
#include <wininet.h>
#include <stdio.h>

#include "net.h"
#include "utils.h"

typedef BOOL(WINAPI *InternetCrackUrlA_t)(LPCSTR, DWORD, DWORD, LPURL_COMPONENTSA);
static InternetCrackUrlA_t InternetCrackUrlA_orig = NULL;

static BOOL WINAPI InternetCrackUrlA_hook(
	LPCSTR lpszUrl,
	DWORD dwUrlLength,
	DWORD dwFlags,
	LPURL_COMPONENTSA lpUrlComponents
) {
	BOOL res = InternetCrackUrlA_orig(lpszUrl, dwUrlLength, dwFlags, lpUrlComponents);

#if _DEBUG
	printf("%s(lpszUrl=%s", __FUNCTION__, lpszUrl);

	if (lpUrlComponents != NULL) {
		printf(", scheme: %d, port: %d", lpUrlComponents->nScheme, lpUrlComponents->nPort);
	}

	printf(") = %d\n", res);
#endif

	if (res && lpUrlComponents != NULL) {
		lpUrlComponents->nScheme = INTERNET_SCHEME_HTTP;
		lpUrlComponents->nPort = 80;
	}

	return res;
}

void net_init() {
	HMODULE mod = GetModuleHandleA("Wininet.dll");

	if (mod == NULL) {
#if _DEBUG
		fprintf(stderr, "Wininet is not loaded\n");
#endif
		return;
	}

	hook_function(mod, "InternetCrackUrlA", (void *) &InternetCrackUrlA_hook, (void **) &InternetCrackUrlA_orig);
}
