#include "pch.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            OutputDebugStringA("[SimpleDLL] DLL_PROCESS_ATTACH");
            MessageBox(NULL, TEXT("I have been injected!"), TEXT("Simple DLL"), MB_OK | MB_ICONASTERISK);
            break;
        case DLL_THREAD_ATTACH:
            OutputDebugStringA("[SimpleDLL] DLL_THREAD_ATTACH");
            break;
        case DLL_THREAD_DETACH:
            OutputDebugStringA("[SimpleDLL] DLL_THREAD_DETACH");
            break;
        case DLL_PROCESS_DETACH:
            OutputDebugStringA("[SimpleDLL] DLL_PROCESS_DETACH");
            MessageBox(NULL, TEXT("I have been ejected!"), TEXT("Simple DLL"), MB_OK | MB_ICONASTERISK);
            break;
    }
    return TRUE;
}
