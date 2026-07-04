// Texture memory management and texture upload - OpenGL version

#include "pch.h"

// OpenGL texture ID storage
GLuint g_opengl_texture_ids[MAXTEXTURES] = {0};
static int g_texture_initialized[MAXTEXTURES] = {0};

// Convert Glide texture format to OpenGL format
static GLenum get_gl_format(int glide_format)
{
    switch (glide_format & X_FORMATMASK) {
        case X_RGBA5551:
        case X_RGBA4444:
            return GL_RGBA;
        case X_RGB565:
            return GL_RGB;
        case X_I8:
            return GL_LUMINANCE;
        case X_IA88:
            return GL_LUMINANCE_ALPHA;
        default:
            return GL_RGBA;
    }
}

// Convert Glide texture internal format to OpenGL internal format
static GLenum get_gl_internal_format(int glide_format)
{
    switch (glide_format & X_FORMATMASK) {
        case X_RGBA5551:
            return GL_RGB5_A1;
        case X_RGBA4444:
            return GL_RGBA4;
        case X_RGB565:
            return GL_RGB565;
        case X_I8:
            return GL_LUMINANCE8;
        case X_IA88:
            return GL_LUMINANCE8_ALPHA8;
        default:
            return GL_RGBA;
    }
}

// Convert Glide texture format to OpenGL data type
static GLenum get_gl_type(int glide_format)
{
    switch (glide_format & X_FORMATMASK) {
        case X_RGBA5551:
        case X_RGBA4444:
            return GL_UNSIGNED_SHORT_1_5_5_5_REV;
        case X_RGB565:
            return GL_UNSIGNED_SHORT_5_6_5_REV;
        case X_I8:
        case X_IA88:
            return GL_UNSIGNED_BYTE;
        default:
            return GL_UNSIGNED_BYTE;
    }
}

// Convert RGBA8888 data to other formats
static void convert_texture_data(unsigned char* src, unsigned char* dst, int width, int height, int glide_format)
{
    int i;
    int num_pixels = width * height;
    
    switch (glide_format & X_FORMATMASK) {
        case X_RGBA5551:
        case X_RGBA4444:
        case X_RGB565: {
            unsigned short* dst16 = (unsigned short*)dst;
            for (i = 0; i < num_pixels; i++) {
                unsigned char r = src[i * 4 + 0];
                unsigned char g = src[i * 4 + 1];
                unsigned char b = src[i * 4 + 2];
                unsigned char a = src[i * 4 + 3];
                
                switch (glide_format & X_FORMATMASK) {
                    case X_RGBA5551:
                        dst16[i] = ((r & 0xF8) << 8) | ((g & 0xF8) << 3) | ((b & 0xF8) >> 2) | ((a & 0x80) >> 15);
                        break;
                    case X_RGB565:
                        dst16[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
                        break;
                    case X_RGBA4444:
                        dst16[i] = ((r & 0xF0) << 8) | ((g & 0xF0) << 4) | (b & 0xF0) | (a >> 4);
                        break;
                }
            }
            break;
        }
        case X_I8: {
            unsigned char* dst8 = (unsigned char*)dst;
            for (i = 0; i < num_pixels; i++) {
                // Convert RGB to luminance
                dst8[i] = (unsigned char)((src[i * 4 + 0] * 77 + src[i * 4 + 1] * 150 + src[i * 4 + 2] * 29) >> 8);
            }
            break;
        }
        case X_IA88: {
            unsigned short* dst16 = (unsigned short*)dst;
            for (i = 0; i < num_pixels; i++) {
                unsigned char r = src[i * 4 + 0];
                unsigned char g = src[i * 4 + 1];
                unsigned char b = src[i * 4 + 2];
                unsigned char a = src[i * 4 + 3];
                unsigned char luminance = (unsigned char)((r * 77 + g * 150 + b * 29) >> 8);
                dst16[i] = (luminance << 8) | a;
            }
            break;
        }
        default:
            // No conversion needed for RGBA8888
            if (dst != src) {
                memcpy(dst, src, width * height * 4);
            }
            break;
    }
}

// Create OpenGL texture
int xgl_createtexture(int handle, int format, int width, int height)
{
    if (handle <= 0 || handle >= MAXTEXTURES) {
        xgl_log("Invalid texture handle: %d\n", handle);
        return -1;
    }
    
    // Generate OpenGL texture ID
    glGenTextures(1, &g_opengl_texture_ids[handle]);
    g_texture_initialized[handle] = 1;
    
    return 0;
}

// Load texture level to OpenGL
int xgl_loadtexturelevel(int handle, int level, char* data)
{
    if (!g_texture_initialized[handle]) {
        xgl_log("Texture not initialized: %d\n", handle);
        return 0;
    }
    
    // Get texture info
    int glide_format, memformat, width, height;
    if (xgl_gettextureinfo(handle, &glide_format, &memformat, &width, &height) != 0) {
        return 0;
    }
    
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, g_opengl_texture_ids[handle]);
    
    // Calculate level dimensions
    int level_width = width >> level;
    int level_height = height >> level;
    if (level_width < 1) level_width = 1;
    if (level_height < 1) level_height = 1;
    
    // Allocate buffer for converted data
    int level_size = level_width * level_height * 4;
    unsigned char* converted_data = (unsigned char*)x_alloc(level_size);
    
    // Convert texture data
    convert_texture_data((unsigned char*)data, converted_data, level_width, level_height, glide_format);
    
    // Upload texture to OpenGL
    glTexImage2D(GL_TEXTURE_2D, level, get_gl_internal_format(glide_format),
                 level_width, level_height, 0,
                 get_gl_format(glide_format), get_gl_type(glide_format), converted_data);
    
    x_free(converted_data);
    
    return 2 - ((glide_format & X_MIPMAP) < 1);
}

// Free OpenGL texture
void xgl_freetexture(int handle)
{
    if (g_texture_initialized[handle]) {
        glDeleteTextures(1, &g_opengl_texture_ids[handle]);
        g_opengl_texture_ids[handle] = 0;
        g_texture_initialized[handle] = 0;
    }
}

// Clear texture memory (free all textures)
void xgl_cleartexmem(void)
{
    for (int i = 1; i < MAXTEXTURES; i++) {
        if (g_texture_initialized[i]) {
            glDeleteTextures(1, &g_opengl_texture_ids[i]);
            g_opengl_texture_ids[i] = 0;
            g_texture_initialized[i] = 0;
        }
    }
}

// Get texture info
int xgl_gettextureinfo(int handle, int* format, int* memformat, int* width, int* height)
{
    // This would need to track texture info separately
    // For now, return basic info
    if (format) *format = 0;
    if (memformat) *memformat = 0;
    if (width) *width = 0;
    if (height) *height = 0;
    return 0;
}

// Open texture data for modification
unsigned char* xgl_opentexturedata(int handle)
{
    // Would need to track original data for dynamic textures
    return NULL;
}

// Close texture data after modification
void xgl_closetexturedata(int handle)
{
    // Reload texture if it was modified
}
