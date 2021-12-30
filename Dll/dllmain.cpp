﻿#include <thread>
#include <cstdint>
#include <fstream>
#include <windows.h>
#include <gl/GL.h>
#include "glext.h"
#include <detours/detours.h>
#pragma comment(lib, "opengl32.lib")

auto output = std::ofstream("log.txt");
auto glEnd_origin = glEnd;
auto glClear_origin = glClear;
int glEndCount = 0;

void Draw(float x, float y, float z)
{
	static auto glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	static auto glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");

    int currentTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &currentTexture);

    glActiveTextureARB(GL_TEXTURE0);
    bool texture0 = glIsEnabled(GL_TEXTURE_2D);
    if (texture0)
        glDisable(GL_TEXTURE_2D);

    glActiveTextureARB(GL_TEXTURE1);
    bool texture1 = glIsEnabled(GL_TEXTURE_2D);
    if (texture1)
        glDisable(GL_TEXTURE_2D);

    bool cull = glIsEnabled(GL_CULL_FACE);
    if (cull)
        glDisable(GL_CULL_FACE);

    double color[4];
    glGetDoublev(GL_CURRENT_COLOR, color);

    glDepthRange(0, 0.1f);
    {
        float width = 10;
        float height = 10;
		glBegin(GL_QUADS);
        glColor3f(1.f, 0.0f, 0.f);
		glVertex3f(x, y, z);
        glVertex3f(x + width, y, z);
		glVertex3f(x + width, y, z + height);
		glVertex3f(x, y, z + height);

		glVertex3f(x, y, z);
        glVertex3f(x, y + width, z);
		glVertex3f(x, y + width, z + height);
		glVertex3f(x, y, z + height);

		glEnd();
    }
    glDepthRange(0, 1);

	glColor4dv(color);

    if (cull)
        glEnable(GL_CULL_FACE);

    if (texture1)
    {
		glActiveTextureARB(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
    }

    if (texture0)
    {
		glActiveTextureARB(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
    }

    glActiveTexture(currentTexture);
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

void Tick()
{
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

        output << "team: " << team << " pos: (" << x << ", " << y << ", " << z << ")" << std::endl;

        if (team == -1 || team == selfTeam)
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