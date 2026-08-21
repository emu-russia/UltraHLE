// Loads the OpenGL extension entry points used by the library.
// The GL context must be current when glcompat_load() is called.
// All of these extensions are supported by every PC GPU driver since ~1999.

#include "pch.h"

PFNGLACTIVETEXTUREARBPROC    glActiveTextureARB;
PFNGLMULTITEXCOORD4FARBPROC  glMultiTexCoord4fARB;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
PFNGLBLENDFUNCSEPARATEEXTPROC glBlendFuncSeparateEXT;
PFNGLFOGCOORDFEXTPROC        glFogCoordfEXT;

int glcompat_load(void)
{
	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
	glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB");
	glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
	glBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC)wglGetProcAddress("glBlendFuncSeparateEXT");
	glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC)wglGetProcAddress("glFogCoordfEXT");

	if (!glActiveTextureARB || !glMultiTexCoord4fARB)
	{
		// No multitexture: the library falls back to single-texture mode.
		glActiveTextureARB = NULL;
		glMultiTexCoord4fARB = NULL;
		glClientActiveTextureARB = NULL;
	}

	return 0;
}
