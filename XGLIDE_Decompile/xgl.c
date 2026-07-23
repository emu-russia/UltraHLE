#include "pch.h"

// OpenGL extension function pointers are provided by GLEW
// glew.h includes glext.h which defines these function pointer types and variables

// OpenGL context state
static HGLRC hRC = NULL;
static HDC hDC = NULL;
static HWND hWnd = NULL;
static int g_width = 0;
static int g_height = 0;
static int g_buffers = 0;
static int g_vsync = 0;

// Current texture units state
static int current_textures[2] = {0, 0};
static int current_texture_mode[2] = {0, 0};

// Vertex arrays for OpenGL
#define MAX_VERTICES 256
static GLfloat vertex_array[MAX_VERTICES * 3];
static GLfloat color_array[MAX_VERTICES * 4];
static GLfloat texcoord0_array[MAX_VERTICES * 2];
static GLfloat texcoord1_array[MAX_VERTICES * 2];
static int vertex_count = 0;
static int current_primitive = 0;

// Matrix state
static float modelview_matrix[16] = {
    1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1
};
static float projection_matrix[16] = {
    1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1
};

// Logging
void xgl_log(char* txt, ...)
{
    static char buf[0x100];
    va_list va;
    va_start(va, txt);
    vsprintf(buf, txt, va);
    x_log(buf);
}

void xgl_fatal(char* txt, ...)
{
    static char buf[0x100];
    va_list va;
    va_start(va, txt);
    vsprintf(buf, txt, va);
    x_fatal(buf);
}

// Memory allocation
void* xgl_alloc(int size)
{
    return x_alloc(size);
}

void* xgl_allocfast(int size)
{
    return x_allocfast(size);
}

void* xgl_realloc(void* p, int newsize)
{
    return x_realloc(p, newsize);
}

void xgl_free(void* blk)
{
    x_free(blk);
}

void xgl_fastfpu(int fast)
{
    x_fastfpu(fast);
}

// Timer functions
void xgl_timereset(void)
{
    x_timereset();
}

int xgl_timems(void)
{
    return x_timems();
}

int xgl_timeus(void)
{
    return x_timeus();
}

void xgl_sleep(int ms)
{
    x_sleep(ms);
}

// Initialize OpenGL
int xgl_init(void* hdc, void* hwnd, int width, int height)
{
    PIXELFORMATDESCRIPTOR pfd;
    int pixel_format;
    
    hDC = (HDC)hdc ? hdc : GetDC(hwnd);
    hWnd = (HWND)hwnd;
    g_width = width;
    g_height = height;
    
    // Set up pixel format
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    pixel_format = ChoosePixelFormat(hDC, &pfd);
    if (!pixel_format) {
        xgl_log("Failed to choose pixel format\n");
        return -1;
    }
    
    if (!SetPixelFormat(hDC, pixel_format, &pfd)) {
        xgl_log("Failed to set pixel format\n");
        return -1;
    }
    
    // Create OpenGL context
    hRC = wglCreateContext(hDC);
    if (!hRC) {
        xgl_log("Failed to create OpenGL context\n");
        return -1;
    }
    
    if (!wglMakeCurrent(hDC, hRC)) {
        xgl_log("Failed to make context current\n");
        return -1;
    }
    
    // Initialize GLEW for extension loading
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        xgl_log("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        return -1;
    }
    
    // Check for multitexture extension
    if (!GLEW_ARB_multitexture) {
        xgl_log("Multitexture extensions not available\n");
        // Continue without multitexture support
    }
    
    // Initialize OpenGL state
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Enable alpha test
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f);
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable dithering
    glEnable(GL_DITHER);
    
    // Initialize texture units
    if (GLEW_ARB_multitexture) {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        
        glActiveTextureARB(GL_TEXTURE0_ARB);
    } else {
        glEnable(GL_TEXTURE_2D);
    }
    
    xgl_log("OpenGL initialized: %s\n", glGetString(GL_VERSION));
    xgl_log("Renderer: %s\n", glGetString(GL_RENDERER));
    
    return 0;
}

void xgl_deinit(void)
{
    if (hRC) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hRC);
        hRC = NULL;
    }
    if (hDC) {
        ReleaseDC(hWnd, hDC);
        hDC = NULL;
    }
    hWnd = NULL;
}

void xgl_resize(int width, int height)
{
    g_width = width;
    g_height = height;
    
    if (hRC) {
        wglMakeCurrent(hDC, hRC);
        glViewport(0, 0, width, height);
        
        // Update projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
    }
}

void xgl_select_context(void)
{
    if (hRC) {
        wglMakeCurrent(hDC, hRC);
    }
}

void xgl_clear(int writecolor, int writedepth, float cr, float cg, float cb)
{
    glClearColor(cr, cg, cb, 1.0f);
    
    GLbitfield mask = 0;
    if (writecolor) mask |= GL_COLOR_BUFFER_BIT;
    if (writedepth) mask |= GL_DEPTH_BUFFER_BIT;
    
    glClear(mask);
}

int xgl_readfb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
    GLenum format;
    GLsizei rowbytes;
    
    if ((uint8_t)fb == X_FB_RGB565) {
        format = GL_RGB;
        rowbytes = 2 * xs;
    } else if ((uint8_t)fb == X_FB_RGBA8888) {
        format = GL_RGBA;
        rowbytes = 4 * xs;
    } else {
        return 1;
    }
    
    if (rowbytes > bufrowlen) return 1;
    
    // Read from front or back buffer
    glReadBuffer((fb & X_FB_FRONT) ? GL_FRONT : GL_BACK);
    
    // Flip Y coordinate (OpenGL origin is bottom-left)
    glReadPixels(x, g_height - y - ys, xs, ys, format, GL_UNSIGNED_BYTE, buffer);
    
    return 0;
}

int xgl_writefb(int fb, int x, int y, int xs, int ys, char* buffer, int bufrowlen)
{
    // Not implemented - write to framebuffer is not commonly supported in OpenGL
    return 1;
}

void xgl_bufferswap(void)
{
    if (g_buffers > 1) {
        SwapBuffers(hDC);
    } else {
        glFlush();
    }
}
