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
#include <glm/glm.hpp>
#include <detours/detours.h>
#include "graphics.h"
#include "memory.h"
#include "log.h"

#pragma comment(lib, "opengl32.lib")

auto output = std::ofstream("log.txt");
auto glEnd_origin = glEnd;
auto glClear_origin = glClear;
int glEndCount = 0;

void Tick()
{
    auto array = GetPlayerArray().value_or(0);
    if (array == 0)
        return;

    uint32_t player = array + 0x324;
    int playerTeam = GetTeam(player);
    uint32_t enemy = 0;

    for (int i = 2; i < 32; ++i)
    {
        uint32_t other = array + i * 0x324;
        auto otherPosition = GetPosition(other);
        int otherTeam = GetTeam(other);
        bool otherAlive = IsAlive(other);

        output << std::format("{}\tteam: {:3}\talive: {}\tpos: {}\n",
            std::chrono::system_clock::now(), otherTeam, otherAlive, otherPosition);

        if (otherTeam == -1 || otherTeam == playerTeam || !otherAlive)
            continue;
        if (enemy == 0)
        {
			enemy = other;
            Draw(otherPosition, glm::vec3{ 0,0,1 });
        }
        else
        {
            Draw(otherPosition, glm::vec3{ 1,0,0 });
        }
    }

    if (enemy == 0)
        return;

    glm::vec3 cameraPos = CameraPosition();
    glm::vec3 enemyPos = GetPosition(enemy);
    glm::vec3 offset = enemyPos - cameraPos;

    float yaw = glm::degrees(std::atan2(offset.y, offset.x));
    float pitch = -glm::degrees(std::atan(offset.z / std::sqrt(offset.x * offset.x + offset.y * offset.y)));
    CameraYaw() = yaw;
    CameraPitch() = pitch;

    output << std::format("camera pos: {} yaw {} pitch {}\n", cameraPos, yaw, pitch);
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