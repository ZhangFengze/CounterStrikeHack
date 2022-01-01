#pragma once
#include <optional>

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

std::optional<uint32_t> GetPlayerArray()
{
    uint32_t dll = (uint32_t)GetModuleHandle("hw.dll");
    return Get<uint32_t>({ dll + 0x00843D60 });
}

inline float* GetPosition(uint32_t player)
{
    return (float*)(player + 0x88);
}

inline int GetTeam(uint32_t player)
{
    return Get<int>({ player + 0x7C, 0x1C8 }).value_or(-1);
}

inline bool IsAlive(uint32_t player)
{
    return Get<int>({ player + 0x13C }).value_or(0);
}