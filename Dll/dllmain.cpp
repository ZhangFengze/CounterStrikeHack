#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <format>
#include <chrono>
#include <windows.h>
#include <gl/GL.h>
#include "glext.h"
#include <detours/detours.h>
#include "graphics.h"
#include "memory.h"

#pragma comment(lib, "opengl32.lib")

auto output = std::ofstream("log.txt");
auto glEnd_origin = glEnd;
auto glClear_origin = glClear;
int glEndCount = 0;

std::string matrix(float* matrix)
{
    std::ostringstream stream;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            stream << std::format("{:12.6f}\t", matrix[row * 4 + col]);
        }
        stream << std::endl;
    }
    return stream.str();
}

void Tick()
{
    float mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    output << "matrix: \n" << matrix(mv) << std::endl;

    auto array = GetPlayerArray().value_or(0);
    if (array == 0)
        return;

    uint32_t self = array + 0x324;
    int selfTeam = GetTeam(self);

    for (int i = 2; i < 32; ++i)
    {
        uint32_t player = array + i * 0x324;

        float* position = GetPosition(player);
        float x = position[0];
        float y = position[1];
        float z = position[2];

        int team = GetTeam(player);
        bool alive = IsAlive(player);

        output << std::format("{}\tteam: {:3}\talive: {}\tpos: ({:12.6}, {:12.6}, {:12.6})\n",
            std::chrono::system_clock::now(), team, alive, x, y, z);

        if (team == -1 || team == selfTeam || !alive)
            continue;
        Draw(x, y, z);
    }
}

void APIENTRY glEnd_mine(void)
{
    ++glEndCount;
    glEnd_origin();
    if (glEndCount == 2)
        Tick();
}

void APIENTRY glClear_mine(GLbitfield mask)
{
    glClear_origin(mask);
    glEndCount = 0;
}

 __declspec(dllexport) INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved)
{

    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hDLL);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)glEnd_origin, glEnd_mine);
        DetourAttach(&(PVOID&)glClear_origin, glClear_mine);
        DetourTransactionCommit();
    }
    break;
    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)glEnd_origin, glEnd_mine);
        DetourDetach(&(PVOID&)glClear_origin, glClear_mine);
        DetourTransactionCommit();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}