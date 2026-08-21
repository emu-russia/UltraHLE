// glcompat.h
//
// OpenGL 1.1 provides the fixed-function pipeline we need, but a few
// extensions are required as well:
//
//   GL_ARB_multitexture          - second texture unit (dual texture passes)
//   GL_EXT_blend_func_separate   - separate RGB/alpha blend factors
//   GL_ARB_texture_env_combine   - per-stage combiner functions (X combine modes)
//
// All of these have been supported by every PC GPU driver since ~1999, so in
// practice they are always present. They are loaded through wglGetProcAddress
// (the GL 1.1 core functions come from opengl32.lib directly).
//
// The GL_ARB_texture_env_combine constants are plain GLenum values passed to
// the core glTexEnvi()/glTexEnvfv() functions, so only the constants need to
// be defined here; no function pointers are required for the combiner.

#ifndef _GLCOMPAT_H_
#define _GLCOMPAT_H_

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// GL_ARB_texture_env_combine constants (not present in the GL 1.1 gl.h)
// ---------------------------------------------------------------------------

#ifndef GL_COMBINE
#define GL_COMBINE                        0x8570
#endif
#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB                    0x8571
#endif
#ifndef GL_COMBINE_ALPHA
#define GL_COMBINE_ALPHA                  0x8572
#endif
#ifndef GL_RGB_SCALE
#define GL_RGB_SCALE                      0x8573
#endif
#ifndef GL_ADD_SIGNED
#define GL_ADD_SIGNED                     0x8574
#endif
#ifndef GL_INTERPOLATE
#define GL_INTERPOLATE                    0x8575
#endif
#ifndef GL_CONSTANT
#define GL_CONSTANT                       0x8576
#endif
#ifndef GL_PRIMARY_COLOR
#define GL_PRIMARY_COLOR                  0x8577
#endif
#ifndef GL_PREVIOUS
#define GL_PREVIOUS                       0x8578
#endif
#ifndef GL_SOURCE0_RGB
#define GL_SOURCE0_RGB                    0x8580
#endif
#ifndef GL_SOURCE1_RGB
#define GL_SOURCE1_RGB                    0x8581
#endif
#ifndef GL_SOURCE2_RGB
#define GL_SOURCE2_RGB                    0x8582
#endif
#ifndef GL_SOURCE0_ALPHA
#define GL_SOURCE0_ALPHA                  0x8588
#endif
#ifndef GL_SOURCE1_ALPHA
#define GL_SOURCE1_ALPHA                  0x8589
#endif
#ifndef GL_SOURCE2_ALPHA
#define GL_SOURCE2_ALPHA                  0x858a
#endif
#ifndef GL_OPERAND0_RGB
#define GL_OPERAND0_RGB                   0x8590
#endif
#ifndef GL_OPERAND1_RGB
#define GL_OPERAND1_RGB                   0x8591
#endif
#ifndef GL_OPERAND2_RGB
#define GL_OPERAND2_RGB                   0x8592
#endif
#ifndef GL_OPERAND0_ALPHA
#define GL_OPERAND0_ALPHA                 0x8598
#endif
#ifndef GL_OPERAND1_ALPHA
#define GL_OPERAND1_ALPHA                 0x8599
#endif
#ifndef GL_OPERAND2_ALPHA
#define GL_OPERAND2_ALPHA                 0x859a
#endif
#ifndef GL_SUBTRACT
#define GL_SUBTRACT                       0x84e7
#endif

// GL_BGRA (GL_EXT_bgra, needed for 1555/4444 texture uploads)
#ifndef GL_BGRA
#define GL_BGRA                           0x80e1
#endif

// GL_CLAMP_TO_EDGE (GL 1.2 / GL_SGIS_texture_edge_clamp)
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE                  0x812f
#endif

// GL_EXT_fog_coord constants (not present in the GL 1.1 gl.h)
#ifndef GL_FOG_COORDINATE_SOURCE_EXT
#define GL_FOG_COORDINATE_SOURCE_EXT      0x8450
#endif
#ifndef GL_FOG_COORDINATE_EXT
#define GL_FOG_COORDINATE_EXT             0x8451
#endif

// ---------------------------------------------------------------------------
// GL_ARB_multitexture function pointers
// ---------------------------------------------------------------------------

// gl.h (GL 1.1) does not define the extension function pointer types
#ifndef PFNGLACTIVETEXTUREARBPROC
typedef void(APIENTRY* PFNGLACTIVETEXTUREARBPROC)(GLenum);
#endif
#ifndef PFNGLMULTITEXCOORD4FARBPROC
typedef void(APIENTRY* PFNGLMULTITEXCOORD4FARBPROC)(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
#endif
#ifndef PFNGLCLIENTACTIVETEXTUREARBPROC
typedef void(APIENTRY* PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum);
#endif

/** Extension entry point for glActiveTextureARB (GL_ARB_multitexture). */
extern PFNGLACTIVETEXTUREARBPROC    glActiveTextureARB;    // active texture unit
/** Extension entry point for glMultiTexCoord4fARB (GL_ARB_multitexture). */
extern PFNGLMULTITEXCOORD4FARBPROC  glMultiTexCoord4fARB;  // per-vertex texcoord for unit N
/** Extension entry point for glClientActiveTextureARB (GL_ARB_multitexture). */
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;

// ---------------------------------------------------------------------------
// GL_EXT_blend_func_separate / GL_EXT_fog_coord function pointers
// ---------------------------------------------------------------------------

// gl.h (GL 1.1) does not define these two extension types
#ifndef PFNGLBLENDFUNCSEPARATEEXTPROC
typedef void(APIENTRY* PFNGLBLENDFUNCSEPARATEEXTPROC)(GLenum, GLenum, GLenum, GLenum);
#endif
#ifndef PFNGLFOGCOORDFEXTPROC
typedef void(APIENTRY* PFNGLFOGCOORDFEXTPROC)(GLfloat);
#endif

/** Extension entry point for glBlendFuncSeparateEXT (GL_EXT_blend_func_separate). */
extern PFNGLBLENDFUNCSEPARATEEXTPROC glBlendFuncSeparateEXT;
/** Extension entry point for glFogCoordfEXT (GL_EXT_fog_coord). */
extern PFNGLFOGCOORDFEXTPROC         glFogCoordfEXT;

// wglSwapIntervalEXT (WGL_EXT_swap_control)
#ifndef WGL_SWAP_INTERVAL_EXT
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
#endif

// Texture units (GL_ARB_multitexture)
#define GL_TEXTURE0_ARB 0x84c0
#define GL_TEXTURE1_ARB 0x84c1
#define GL_TEXTURE2_ARB 0x84c2

/**
 * Loads all OpenGL extension entry points used by the library.
 * @return 0 on success.
 */
// Load all extension entry points. Returns 0 on success.
// The GL context must be current when this is called.
int glcompat_load(void);

#ifdef __cplusplus
}
#endif

#endif // _GLCOMPAT_H_
