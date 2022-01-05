#pragma once
#include <optional>
#include <glm/glm.hpp>

#if 0
const int playerArrayOffset = 0x00843D60;
const int cameraPitchOffset = 0x108AEC4;
const int cameraYawOffset = 0x108AEC8;
const int cameraPositionOffset = 0x108AEE8;
#else
const int playerArrayOffset = 0x00809820;
const int cameraPitchOffset = 0x108AEC4 - 0x34520;
const int cameraYawOffset = 0x108AEC8 - 0x34520;
const int cameraPositionOffset = 0x108AEE8 - 0x34520;
#endif

template<typename T>
std::optional<T> Get(std::initializer_list<uint32_t> offsets)
{
    if (offsets.size() == 0)
        return std::nullopt;

    auto iter = offsets.begin();
    uint32_t p = *(iter++);
	if (p == 0)
		return std::nullopt;

    while (iter != offsets.end())
    {
        p = *(uint32_t*)p;
        if (p == 0)
            return std::nullopt;
        p += *(iter++);
    }
    return *(T*)p;
}

inline std::optional<uint32_t> GetPlayerArray()
{
    uint32_t dll = (uint32_t)GetModuleHandle("hw.dll");
    return Get<uint32_t>({ dll + playerArrayOffset });
}

inline glm::vec3 GetPosition(uint32_t player)
{
    auto p = (float*)(player + 0x88);
    return { p[0],p[1],p[2] };
}

inline int GetTeam(uint32_t player)
{
    return Get<int>({ player + 0x7C, 0x1C8 }).value_or(-1);
}

inline bool IsAlive(uint32_t player)
{
    return Get<int>({ player + 0x13C }).value_or(0);
}

float& CameraPitch()
{
    uint32_t dll = (uint32_t)GetModuleHandle("hw.dll");
    return *(float*)(dll + cameraPitchOffset);
}

float& CameraYaw()
{
    uint32_t dll = (uint32_t)GetModuleHandle("hw.dll");
    return *(float*)(dll + cameraYawOffset);
}

glm::vec3 CameraPosition()
{
    uint32_t dll = (uint32_t)GetModuleHandle("hw.dll");
    float* p = (float*)(dll + cameraPositionOffset);
    return { p[0],p[1],p[2] };
}
