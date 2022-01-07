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
#include <glm/gtx/vector_angle.hpp>
#include <detours/detours.h>
#include "graphics.h"
#include "memory.h"
#include "log.h"

#pragma comment(lib, "opengl32.lib")

auto output = std::ofstream("log.txt");
auto glEnd_origin = glEnd;
auto glClear_origin = glClear;
int glEndCount = 0;

struct PlayerInfo
{
    uint32_t p;
    glm::vec3 position;
    float aimYaw;
    float aimPitch;
    float aimAngleDistance;

    PlayerInfo(uint32_t p)
    {
        this->p = p;
        position = GetPosition(p);

        glm::vec3 offset = position - CameraPosition();
		aimYaw = glm::degrees(std::atan2(offset.y, offset.x));
        aimPitch = -glm::degrees(std::atan(offset.z / std::sqrt(offset.x * offset.x + offset.y * offset.y)));

        auto aim = glm::quat{ glm::vec3{0,glm::radians(CameraPitch()),glm::radians(CameraYaw())} }*glm::vec3{ 1,0,0 };
        aimAngleDistance = glm::degrees(glm::angle(aim, glm::normalize(offset)));
    }
};

void Tick()
{
    auto array = GetPlayerArray().value_or(0);
    if (array == 0)
        return;

    uint32_t player = array + 0x324;
    int playerTeam = GetTeam(player);

    std::vector<PlayerInfo> enemies;
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
        enemies.emplace_back(other);
    }


	auto minIt = std::min_element(enemies.begin(), enemies.end(),
		[](const auto& left, const auto& right)
		{
			return left.aimAngleDistance < right.aimAngleDistance;
		});

    for (auto it = enemies.begin(); it != enemies.end(); ++it)
    {
        if (it == minIt && it->aimAngleDistance < 5)
        {
			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{
                CameraYaw() = it->aimYaw;
                CameraPitch() = it->aimPitch;
			}
			Draw(it->position, glm::vec3{ 0,0,1 });
        }
        else
        {
			Draw(it->position, glm::vec3{ 1,0,0 });
        }
    }

	auto aim = glm::quat{ glm::vec3{0,glm::radians(CameraPitch()),glm::radians(CameraYaw())} }*glm::vec3{ 1,0,0 };
    output << std::format("camera pos: {}\tyaw:{}\tpitch:{}\taim:{}\n", CameraPosition(), CameraYaw(), CameraPitch(), aim);
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
        output << "dll attach" << std::endl;
    }
    break;
    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)glEnd_origin, glEnd_mine);
        DetourDetach(&(PVOID&)glClear_origin, glClear_mine);
        DetourTransactionCommit();
        output << "dll detach" << std::endl;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}