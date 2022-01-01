#pragma once
#include <format>
#include <glm/glm.hpp>

namespace std
{
	template <>
	struct formatter<glm::vec3>
	{
		constexpr auto parse(format_parse_context& ctx)
		{
            return end(ctx);
		}

		template <typename FormatContext>
		auto format(const glm::vec3& v, FormatContext& ctx)
		{
			auto&& out = ctx.out();
            return format_to(out, "({:.6},{:.6},{:.6})", v.x, v.y, v.z);
		}
	};
}

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
