// OpenGL mode management - replaces fx.c

#include "pch.h"

// OpenGL state tracking
static int g_opengl_blend_src = -1;
static int g_opengl_blend_dst = -1;
static int g_opengl_depth_test = -1;
static int g_opengl_cull_mode = -1;
static int g_opengl_alpha_test = -1;
static int g_opengl_fog_mode = -1;
static int g_opengl_color_mask = -1;
static int g_opengl_depth_mask = -1;
static int g_opengl_dither = -1;

// Convert Glide blend modes to OpenGL
static void convert_blend_mode(int glide_src, int glide_dst, int* gl_src, int* gl_dst)
{
    switch (glide_src) {
        case X_ZERO: *gl_src = GL_ZERO; break;
        case X_ONE: *gl_src = GL_ONE; break;
        case X_OTHER: *gl_src = GL_SRC_COLOR; break;
        case X_ALPHA: *gl_src = GL_SRC_ALPHA; break;
        case X_OTHERALPHA: *gl_src = GL_SRC_ALPHA; break;
        case X_INVOTHER: *gl_src = GL_ONE_MINUS_SRC_COLOR; break;
        case X_INVALPHA: *gl_src = GL_ONE_MINUS_SRC_ALPHA; break;
        case X_INVOTHERALPHA: *gl_src = GL_ONE_MINUS_SRC_ALPHA; break;
        default: *gl_src = GL_ONE; break;
    }
    
    switch (glide_dst) {
        case X_ZERO: *gl_dst = GL_ZERO; break;
        case X_ONE: *gl_dst = GL_ONE; break;
        case X_OTHER: *gl_dst = GL_DST_COLOR; break;
        case X_ALPHA: *gl_dst = GL_DST_ALPHA; break;
        case X_OTHERALPHA: *gl_dst = GL_DST_ALPHA; break;
        case X_INVOTHER: *gl_dst = GL_ONE_MINUS_DST_COLOR; break;
        case X_INVALPHA: *gl_dst = GL_ONE_MINUS_DST_ALPHA; break;
        case X_INVOTHERALPHA: *gl_dst = GL_ONE_MINUS_DST_ALPHA; break;
        default: *gl_dst = GL_ZERO; break;
    }
}

// Convert Glide depth test to OpenGL
static GLenum convert_depth_test(int glide_test)
{
    switch (glide_test) {
        case X_TESTEQ: return GL_EQUAL;
        case X_TESTNE: return GL_NOTEQUAL;
        case X_TESTGE: return GL_GEQUAL;
        case X_TESTLE: return GL_LEQUAL;
        case X_TESTGT: return GL_GREATER;
        case X_TESTLT: return GL_LESS;
        default: return GL_LESS;
    }
}

// Convert Glide cull mode to OpenGL
static GLenum convert_cull_mode(int glide_flags)
{
    if (glide_flags & X_CULLFRONT) return GL_FRONT;
    if (glide_flags & X_CULLBACK) return GL_BACK;
    return GL_NONE;
}

// Initialize OpenGL mode management
void xgl_mode_init(void)
{
    g_opengl_blend_src = -1;
    g_opengl_blend_dst = -1;
    g_opengl_depth_test = -1;
    g_opengl_cull_mode = -1;
    g_opengl_alpha_test = -1;
    g_opengl_fog_mode = -1;
    g_opengl_color_mask = -1;
    g_opengl_depth_mask = -1;
    g_opengl_dither = -1;
    
    // Initialize OpenGL state
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_FOG);
    glDisable(GL_DITHER);
    
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
}

// Apply mode changes
void xgl_mode_change(void)
{
    xt_rendmode* mode = &g_state[XST].currentmode;
    
    // Blend function
    if (mode->src != g_opengl_blend_src || mode->dst != g_opengl_blend_dst) {
        if (mode->src != X_ZERO || mode->dst != X_ONE) {
            glEnable(GL_BLEND);
            int gl_src, gl_dst;
            convert_blend_mode(mode->src, mode->dst, &gl_src, &gl_dst);
            glBlendFunc(gl_src, gl_dst);
        } else {
            glDisable(GL_BLEND);
        }
        g_opengl_blend_src = mode->src;
        g_opengl_blend_dst = mode->dst;
    }
    
    // Depth test
    if (mode->masktst != g_opengl_depth_test) {
        if (mode->masktst > X_DISABLE) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(convert_depth_test(mode->masktst));
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        g_opengl_depth_test = mode->masktst;
    }
    
    // Culling
    int new_cull = convert_cull_mode(g_state[XST].geometry);
    if (new_cull != g_opengl_cull_mode) {
        if (new_cull != GL_NONE) {
            glEnable(GL_CULL_FACE);
            glCullFace(new_cull);
        } else {
            glDisable(GL_CULL_FACE);
        }
        g_opengl_cull_mode = new_cull;
    }
    
    // Alpha test
    if (mode->alphatest != g_opengl_alpha_test) {
        if (mode->alphatest < 1.0f && mode->alphatest > 0.0f) {
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, mode->alphatest);
        } else {
            glDisable(GL_ALPHA_TEST);
        }
        g_opengl_alpha_test = mode->alphatest;
    }
    
    // Fog
    if (mode->fogtype != g_opengl_fog_mode) {
        if (mode->fogtype != X_DISABLE) {
            glEnable(GL_FOG);
            GLfloat fog_color[4] = {mode->fogcolor[0], mode->fogcolor[1], mode->fogcolor[2], mode->fogcolor[3]};
            glFogfv(GL_FOG_COLOR, fog_color);
            glFogf(GL_FOG_DENSITY, 1.0f / (mode->fogmax - mode->fogmin));
            
            switch (mode->fogtype) {
                case X_LINEAR:
                case X_LINEARADD:
                    glFogi(GL_FOG_MODE, GL_LINEAR);
                    glFogf(GL_FOG_START, mode->fogmin);
                    glFogf(GL_FOG_END, mode->fogmax);
                    break;
                case X_EXPONENTIAL:
                    glFogi(GL_FOG_MODE, GL_EXP);
                    break;
            }
        } else {
            glDisable(GL_FOG);
        }
        g_opengl_fog_mode = mode->fogtype;
    }
    
    // Color mask
    if ((mode->mask & 1) != g_opengl_color_mask) {
        glColorMask((mode->mask & 1) ? GL_TRUE : GL_FALSE,
                    (mode->mask & 1) ? GL_TRUE : GL_FALSE,
                    (mode->mask & 1) ? GL_TRUE : GL_FALSE,
                    (mode->mask & 1) ? GL_TRUE : GL_FALSE);
        g_opengl_color_mask = mode->mask & 1;
    }
    
    // Depth mask
    if (((mode->mask >> 1) & 1) != g_opengl_depth_mask) {
        glDepthMask(((mode->mask >> 1) & 1) ? GL_TRUE : GL_FALSE);
        g_opengl_depth_mask = (mode->mask >> 1) & 1;
    }
    
    // Dither
    if (mode->dither != g_opengl_dither) {
        if (mode->dither) {
            glEnable(GL_DITHER);
        } else {
            glDisable(GL_DITHER);
        }
        g_opengl_dither = mode->dither;
    }
}

// Apply texture mode
void xgl_mode_texturemode(int tmu, int format, int trilin)
{
    GLenum target = (tmu > 0 && GLEW_ARB_multitexture) ? GL_TEXTURE1_ARB : GL_TEXTURE0_ARB;
    glBindTexture(target, g_opengl_texture_ids[g_state[XST].currentmode.text1 + tmu]);
    
    // Mipmap filtering
    if (trilin) {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else if (format & X_MIPMAP) {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, (format & X_NOBILIN) ? GL_NEAREST : GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, (format & X_NOBILIN) ? GL_NEAREST : GL_LINEAR);
    }
    
    // Clamp mode
    GLint clamp_s = (format & X_CLAMP) ? (format & X_CLAMPNOX) ? GL_CLAMP_TO_EDGE : GL_CLAMP : GL_REPEAT;
    GLint clamp_t = (format & X_CLAMP) ? (format & X_CLAMPNOY) ? GL_CLAMP_TO_EDGE : GL_CLAMP : GL_REPEAT;
    
    glTexParameteri(target, GL_TEXTURE_WRAP_S, clamp_s);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, clamp_t);
}

// Load single texture
void xgl_mode_loadtexture(int txtind)
{
    if (txtind <= 0 || txtind >= MAXTEXTURES) return;
    
    if (GLEW_ARB_multitexture) {
        glActiveTextureARB(GL_TEXTURE0_ARB);
    }
    
    glBindTexture(GL_TEXTURE_2D, g_opengl_texture_ids[txtind]);
    xgl_mode_texturemode(0, g_texture[txtind].format, 0);
}

// Load multitexture
void xgl_mode_loadmultitexture(int txtind1, int txtind2)
{
    if (txtind1 <= 0 || txtind1 >= MAXTEXTURES) return;
    if (txtind2 <= 0 || txtind2 >= MAXTEXTURES) return;
    
    if (GLEW_ARB_multitexture) {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, g_opengl_texture_ids[txtind1]);
        xgl_mode_texturemode(0, g_texture[txtind1].format, 0);
        
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_2D, g_opengl_texture_ids[txtind2]);
        xgl_mode_texturemode(1, g_texture[txtind2].format, 0);
        
        glActiveTextureARB(GL_TEXTURE0_ARB);
    } else {
        // Single texture mode if multitexture not supported
        glBindTexture(GL_TEXTURE_2D, g_opengl_texture_ids[txtind1]);
    }
}
