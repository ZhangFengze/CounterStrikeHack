#include <fstream>
#include <windows.h>
#include <gl/GL.h>
#include <detours/detours.h>
#pragma comment(lib, "opengl32.lib")

auto output = std::ofstream("log.txt");
auto glEnd_origin = glEnd;
auto glClear_origin = glClear;
int glEndCount = 0;

void Draw(float x, float y, float z)
{
    bool cull = glIsEnabled(GL_CULL_FACE);
    if (cull)
        glDisable(GL_CULL_FACE);

    double color[4];
    glGetDoublev(GL_CURRENT_COLOR, color);

    bool texture = glIsEnabled(GL_TEXTURE_2D);
    if (texture)
        glDisable(GL_TEXTURE_2D);

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

    if (texture)
        glEnable(GL_TEXTURE_2D);

	glColor4dv(color);

    if (cull)
        glEnable(GL_CULL_FACE);
}

void Tick()
{
    char* dll = (char*)GetModuleHandle("hw.dll");
    output << "dll: " << (void*)dll << std::endl;

    char* pArray = dll + 0x00843D60;
    output << "pArray: " << (void*)pArray << std::endl;

    char* array = (char*)(*((int*)pArray));
    output << "array: " << (void*)array << std::endl;

    if (!array)
        return;

    for (int i = 2; i < 32; ++i)
    {
        char* player = array + i * 0x324;

        float* position = (float*)(player + 0x88);
        float x = position[0];
        float y = position[1];
        float z = position[2];

        output << "pos: (" << x << ", " << y << ", " << z << ")" << std::endl;
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