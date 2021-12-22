#include <cstdio>
#include <windows.h>
#include <detours/detours.h>

int main()
{
    char target[MAX_PATH] = R"(D:\SteamLibrary\steamapps\common\Half-Life\hl.exe -game cstrike)";
    char workdir[MAX_PATH] = R"(D:\SteamLibrary\steamapps\common\Half-Life\)";
    char dll[MAX_PATH] = R"(D:\Projects\CounterStrikeHack\Release\Dll.dll)";

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    si.cb = sizeof(STARTUPINFO);
    if (!DetourCreateProcessWithDll(NULL, target, NULL,
        NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, workdir,
        &si, &pi, dll, NULL))
    {
        printf("err");
    }
    else
    {
        printf("ok");
    }
    return 0;
}