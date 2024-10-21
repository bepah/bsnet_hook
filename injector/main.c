#include <Windows.h>

#include <stdio.h>

static const WCHAR *gDllName = L"bsnet_hook.dll";

static int inject_process(const WCHAR *dllPath, HANDLE process) {
	HMODULE mod = NULL;
	FARPROC loadLibrary = NULL;
	HANDLE thread = NULL;

	void *mem = NULL;
	SIZE_T memSize;

	wprintf(L"Injecting %s\n", dllPath);

	mod = GetModuleHandleW(L"kernel32.dll");

	if (mod == NULL) {
		fprintf(stderr, "Failed to find kernel32.dll\n");
		return 1;
	}

	loadLibrary = GetProcAddress(mod, "LoadLibraryW");

	if (loadLibrary == NULL) {
		fprintf(stderr, "Failed to find LoadLibraryW\n");
		return 1;
	}

	memSize = (wcslen(dllPath) + 1) * sizeof(WCHAR);

	mem = VirtualAllocEx(process, NULL, memSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (mem == NULL) {
		fprintf(stderr, "Failed to allocate memory inside of process\n");
		return 1;
	}

	if (!WriteProcessMemory(process, mem, dllPath, memSize, NULL)) {
		fprintf(stderr, "Failed to write memory inside of process\n");
		VirtualFreeEx(process, mem, 0, MEM_RELEASE);
		return 1;
	}

	thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE) loadLibrary, mem, 0, NULL);

	if (thread == NULL) {
		fprintf(stderr, "Failed to create remote thread\n");
		VirtualFreeEx(process, mem, 0, MEM_RELEASE);
		return 1;
	}

	WaitForSingleObject(thread, INFINITE);

	VirtualFreeEx(process, mem, 0, MEM_RELEASE);
	CloseHandle(thread);

	printf("Injected sucessfully\n");

	return 0;
}

static int start_process(const WCHAR *dllPath, const char *exePath) {
	BOOL err;

	STARTUPINFOA startupInfo;
	PROCESS_INFORMATION procInfo;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	err = CreateProcessA(
		exePath,
		"/START",
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		NULL,
		&startupInfo,
		&procInfo
	);

	if (err == FALSE) {
		fprintf(stderr, "Failed to create process 0x%08x\n", GetLastError());
		return 1;
	}

	if (inject_process(dllPath, procInfo.hProcess)) {
		fprintf(stderr, "Failed to inject dll\n");
		TerminateProcess(procInfo.hProcess, 0);
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);
		return 1;
	}

	ResumeThread(procInfo.hThread);

	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);

	return 0;
}

static int get_dll_path(WCHAR *path, size_t pathLen) {
	WCHAR *tmp = NULL;
	DWORD length;
	size_t dllNameLen;
	errno_t err;

	length = GetModuleFileNameW(NULL, path, pathLen);

	if (length == 0) {
		fprintf(stderr, "Failed to GetModuleFileName\n");
		return 1;
	}

	tmp = wcsrchr(path, L'\\');

	if (tmp == NULL) {
		fprintf(stderr, "Invalid path\n");
		return 1;
	}

	dllNameLen = wcslen(gDllName);

	err = wcsncpy_s(tmp + 1, pathLen - (tmp - path) - 1, gDllName, dllNameLen);

	return err != 0;
}

int main(int argc, char *argv[]) {
	WCHAR dllPath[MAX_PATH];

	if (argc < 2) {
		fprintf(stderr, "%s <exe>", argv[0]);
		return 1;
	}

	if (get_dll_path(dllPath, MAX_PATH)) {
		fprintf(stderr, "Failed to get the dll path\n");
		return 1;
	}

	return start_process(dllPath, argv[1]);
}
