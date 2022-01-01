#define _CRT_SECURE_NO_WARNINGS
#include <thread>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <gl/GL.h>
#include "glext.h"
#include <detours/detours.h>
#include <format>
#include "graphics.h"
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

float* GetPosition(uint32_t player)
{
    return (float*)(player + 0x88);
}

int GetTeam(uint32_t player)
{
	uint32_t pBasePlayer = player + 0x7C;
    uint32_t basePlayer = *((uint32_t*)pBasePlayer);
    if (basePlayer == 0)
        return -1;
    return *((int*)(basePlayer + 0x1C8));
}

bool IsAlive(uint32_t player)
{
    return *(int*)(player + 0x13C);
}

void Tick()
{
    float mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    output << "matrix: \n" << matrix(mv) << std::endl;

    uint32_t dll = (uint32_t)GetModuleHandle("hw.dll");
    output << "dll: " << (void*)dll << std::endl;

    uint32_t pArray = dll + 0x00843D60;
    output << "pArray: " << (void*)pArray << std::endl;

    uint32_t array = *((uint32_t*)pArray);
    output << "array: " << (void*)array << std::endl;

    if (array == 0)
        return;

    int selfTeam = GetTeam(array + 0x324);
    output << "self team: " << selfTeam << std::endl;

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