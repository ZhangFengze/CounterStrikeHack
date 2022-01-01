#pragma once
#include <gl/GL.h>
#include "glext.h"

struct DisableTexture
{
    int currentTexture;
    bool texture0;
    bool texture1;

    DisableTexture()
    {
		static auto glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
		static auto glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");

		glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &currentTexture);

		glActiveTextureARB(GL_TEXTURE0);
		texture0 = glIsEnabled(GL_TEXTURE_2D);
		if (texture0)
			glDisable(GL_TEXTURE_2D);

		glActiveTextureARB(GL_TEXTURE1);
		texture1 = glIsEnabled(GL_TEXTURE_2D);
		if (texture1)
			glDisable(GL_TEXTURE_2D);
    }

    ~DisableTexture()
    {
		static auto glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
		static auto glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");

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
};

struct DisableCulling
{
    bool cull;
    DisableCulling()
    {
		cull = glIsEnabled(GL_CULL_FACE);
		if (cull)
			glDisable(GL_CULL_FACE);
    }
    ~DisableCulling()
    {
		if (cull)
			glEnable(GL_CULL_FACE);
	}
};

struct RestoreColor
{
    double color[4];
    RestoreColor()
    {
		glGetDoublev(GL_CURRENT_COLOR, color);
    }
    ~RestoreColor()
    {
		glColor4dv(color);
    }
};

struct ForceDepth0
{
    int ranges[2];
    ForceDepth0()
    {
		glGetIntegerv(GL_DEPTH_RANGE, ranges);
        glDepthRange(0, 0);
    }
    ~ForceDepth0()
    {
        glDepthRange(ranges[0], ranges[1]);
    }
};

inline void Draw(float x, float y, float z)
{
    DisableTexture _;
    DisableCulling __;
    RestoreColor ___;
    ForceDepth0 ____;

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
