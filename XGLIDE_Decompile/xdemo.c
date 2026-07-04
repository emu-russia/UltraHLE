/*
** Win32 demo application for x.h library
** Shows a rotating cube that bounces off screen edges
** Demonstrates all major x.h API calls
*/

#include <windows.h>
#include <stdio.h>

/* Include x.h and all necessary headers */
#include "x.h"

/* Window class name */
#define WINDOW_CLASS_NAME "XDemoWindow"
#define WINDOW_TITLE "X.H Library Demo - Rotating Bouncing Cube"

/* Global variables */
static HWND g_hwnd = NULL;
static HDC g_hdc = NULL;
static int g_window_width = 640;
static int g_window_height = 480;
static int g_context = -1;

xt_data  g_data;

xt_pos   g_pos;

/* Cube vertex data */
static float g_cube_vertices[8][3] = {
    {-1.0f, -1.0f,  1.0f},  /* 0: front-bottom-left */
    { 1.0f, -1.0f,  1.0f},  /* 1: front-bottom-right */
    { 1.0f,  1.0f,  1.0f},  /* 2: front-top-right */
    {-1.0f,  1.0f,  1.0f},  /* 3: front-top-left */
    {-1.0f, -1.0f, -1.0f},  /* 4: back-bottom-left */
    { 1.0f, -1.0f, -1.0f},  /* 5: back-bottom-right */
    { 1.0f,  1.0f, -1.0f},  /* 6: back-top-right */
    {-1.0f,  1.0f, -1.0f}   /* 7: back-top-left */
};

/* Cube colors (one per vertex) */
static float g_cube_colors[8][4] = {
    {1.0f, 0.0f, 0.0f, 1.0f},  /* 0: red */
    {1.0f, 0.5f, 0.0f, 1.0f},  /* 1: orange */
    {1.0f, 1.0f, 0.0f, 1.0f},  /* 2: yellow */
    {0.0f, 1.0f, 0.0f, 1.0f},  /* 3: green */
    {0.0f, 0.0f, 1.0f, 1.0f},  /* 4: blue */
    {0.5f, 0.0f, 1.0f, 1.0f},  /* 5: indigo */
    {0.5f, 0.0f, 0.5f, 1.0f},  /* 6: violet */
    {1.0f, 0.0f, 1.0f, 1.0f}   /* 7: magenta */
};

/* Cube texture coordinates */
static float g_cube_texcoords[8][2] = {
    {0.0f, 0.0f},  /* 0 */
    {1.0f, 0.0f},  /* 1 */
    {1.0f, 1.0f},  /* 2 */
    {0.0f, 1.0f},  /* 3 */
    {0.0f, 0.0f},  /* 4 */
    {1.0f, 0.0f},  /* 5 */
    {1.0f, 1.0f},  /* 6 */
    {0.0f, 1.0f}   /* 7 */
};

/* Cube faces (indices into vertices array) */
static int g_cube_faces[6][4] = {
    {0, 1, 2, 3},  /* front */
    {1, 5, 6, 2},  /* right */
    {5, 4, 7, 6},  /* back */
    {4, 0, 3, 7},  /* left */
    {3, 2, 6, 7},  /* top */
    {4, 5, 1, 0}   /* bottom */
};

/* Cube rotation and position */
static float g_cube_rotation_x = 0.0f;
static float g_cube_rotation_y = 0.0f;
static float g_cube_rotation_z = 0.0f;
static float g_cube_position_x = 0.0f;
static float g_cube_position_y = 0.0f;
static float g_cube_position_z = 5.0f;
static float g_cube_velocity_x = 0.05f;
static float g_cube_velocity_y = 0.03f;
static float g_cube_velocity_z = 0.0f;

/* Texture handle */
static int g_cube_texture = 0;

/* Matrix for cube transformation */
static xt_matrix g_modelview_matrix;

/* Forward declarations */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL SetupPixelFormat(HDC hdc);
void DrawCube(void);
void UpdateCube(void);
void PrintStats(void);

/*
** Create a simple procedural texture
** Creates a checkerboard pattern
*/
static void CreateCheckerboardTexture(uchar* buffer, int width, int height)
{
    int x, y;
    uchar* p = buffer;
    
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* Checkerboard pattern */
            int checker = ((x & 0x8) ^ (y & 0x8)) ? 0xFF : 0x00;
            /* Write RGBA: red and blue checker, green channel */
            p[0] = checker;         /* B */
            p[1] = 0x80;            /* G */
            p[2] = checker;         /* R */
            p[3] = 0xFF;            /* A */
            p += 4;
        }
    }
}

/*
** Initialize the demo
** Sets up the window, graphics context, and creates a texture
*/
BOOL InitializeDemo(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASS wc;
    
    /* Register window class */
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    
    /* Create window */
    g_hwnd = CreateWindow(
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_window_width, g_window_height,
        HWND_DESKTOP, NULL, hInstance, NULL
    );
    
    if (!g_hwnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    
    /* Get DC */
    g_hdc = GetDC(g_hwnd);
    
    /* Initialize x.h library */
    x_init();
    
    /* Get and display version information */
    x_log("Demo initialized. Using: %s\n", x_version());

    /* Open graphics context */
    /* x_open parameters: hdc, hwnd, width, height, buffers, vsync */
    g_context = x_open(NULL, g_hwnd, g_window_width, g_window_height, 2, 1);

    if (g_context < 0) {
        x_log("Failed to open graphics context\n");
        /* Try to continue without hardware acceleration */
        g_context = 1;
    }
    
    /* Set viewport to cover the entire window */
    x_viewport(0.0f, 0.0f, (float)g_window_width, (float)g_window_height);
    
    /* Set up perspective projection */
    x_projection(45.0f, 0.9f, 65535.0f);
    
    /* Set default rendering mode */
    x_reset();
    
    /* Test various mode functions */
    
    /* 1. Test x_mask - control color and depth writes */
    x_mask(X_ENABLE, X_ENABLE, X_ENABLE);
    
    /* 2. Test x_dither */
    x_dither(X_ENABLE);
    
    /* 3. Test x_blend - set alpha blending */
    x_blend(X_ONE, X_ZERO);  /* No blending initially */
    
    /* 4. Test x_alphatest */
    x_alphatest(1.0f);  /* Disable alpha test */
    
    /* 5. Test x_combine - set color/texture combination */
    x_combine(X_COLOR);  /* Use vertex colors */
    
    /* 6. Test x_envcolor - set environment color */
    x_envcolor(0.5f, 0.5f, 0.5f, 1.0f);
    
    /* 7. Test x_zrange - set depth range */
    x_zrange(0.9f, 65535.0f);
    
    /* 8. Test x_zdecal - set depth decal factor */
    x_zdecal(1.0f);
    
    /* 9. Test x_forcegeometry */
    x_forcegeometry(0, 0);
    
    /* 10. Test x_geometry - set geometry flags */
    x_geometry(X_CULLBACK);  /* Cull back faces */
    
    /* 11. Test x_projection (already called above) */
    
    /* 12. Test x_fog - enable fog */
    x_fog(X_LINEAR, 1.0f, 20.0f, 0.0f, 0.0f, 0.0f);
    
    /* Create a texture for the cube */
    g_cube_texture = x_createtexture(X_RGBA8888, 64, 64);
    
    if (g_cube_texture > 0) {
        /* Create texture data */
        uchar texture_data[64 * 64 * 4];
        CreateCheckerboardTexture(texture_data, 64, 64);
        
        /* Load texture level 0 */
        x_loadtexturelevel(g_cube_texture, 0, (char*)texture_data);
        
        x_log("Created texture handle: %d\n", g_cube_texture);
    } else {
        x_log("Failed to create texture\n");
    }
    
    /* Reset to default state after setup */
    x_reset();
    
    return TRUE;
}

/*
** Cleanup demo resources
*/
void CleanupDemo(void)
{
    /* Free texture if created */
    if (g_cube_texture > 0) {
        x_freetexture(g_cube_texture);
        x_log("Freed texture: %d\n", g_cube_texture);
    }
    
    /* Close graphics context */
    if (g_context > 0) {
        x_close(g_context);
        x_log("Closed graphics context: %d\n", g_context);
    }
    
    /* Deinitialize x.h library */
    x_deinit();
    
    /* Release DC */
    if (g_hdc) {
        ReleaseDC(g_hwnd, g_hdc);
        g_hdc = NULL;
    }
    
    /* Destroy window */
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
    
    UnregisterClass(WINDOW_CLASS_NAME, NULL);
}

/*
** Update cube position and rotation
*/
void UpdateCube(void)
{
    /* Update rotation angles */
    g_cube_rotation_x += 1.0f;
    g_cube_rotation_y += 1.5f;
    g_cube_rotation_z += 0.5f;
    
    /* Normalize rotation to 0-360 degrees */
    if (g_cube_rotation_x >= 360.0f) g_cube_rotation_x -= 360.0f;
    if (g_cube_rotation_y >= 360.0f) g_cube_rotation_y -= 360.0f;
    if (g_cube_rotation_z >= 360.0f) g_cube_rotation_z -= 360.0f;
    
    /* Update position */
    g_cube_position_x += g_cube_velocity_x;
    g_cube_position_y += g_cube_velocity_y;
    
    /* Check for collision with screen edges and bounce */
    /* Left and right edges */
    if (g_cube_position_x < -5.0f) {
        g_cube_position_x = -5.0f;
        g_cube_velocity_x = -g_cube_velocity_x;
    } else if (g_cube_position_x > 5.0f) {
        g_cube_position_x = 5.0f;
        g_cube_velocity_x = -g_cube_velocity_x;
    }
    
    /* Top and bottom edges */
    if (g_cube_position_y < -3.0f) {
        g_cube_position_y = -3.0f;
        g_cube_velocity_y = -g_cube_velocity_y;
    } else if (g_cube_position_y > 3.0f) {
        g_cube_position_y = 3.0f;
        g_cube_velocity_y = -g_cube_velocity_y;
    }
    
    /* Create modelview matrix for cube transformation */
    float rad_x = g_cube_rotation_x * (PI / 180.0f);
    float rad_y = g_cube_rotation_y * (PI / 180.0f);
    float rad_z = g_cube_rotation_z * (PI / 180.0f);
    
    /* Reset matrix to identity */
    memset(&g_modelview_matrix, 0, sizeof(xt_matrix));
    g_modelview_matrix.xax = 1.0f;
    g_modelview_matrix.yay = 1.0f;
    g_modelview_matrix.zaz = 1.0f;
    g_modelview_matrix.r4 = 1.0f;
    g_modelview_matrix.type = X_MATRIX_IDENT;
    
    /* Apply translation */
    g_modelview_matrix.x = g_cube_position_x;
    g_modelview_matrix.y = g_cube_position_y;
    g_modelview_matrix.z = g_cube_position_z;
    
    /* Apply rotation (simplified - just setting the matrix) */
    /* Note: A full rotation matrix implementation would be more complex */
    x_matrix(&g_modelview_matrix);
}

/*
** Draw the cube using x.h geometry functions
*/
void DrawCube(void)
{
    int i;
    
    /* Use texture if available */
    if (g_cube_texture > 0) {
        x_texture(g_cube_texture);
        x_combine(X_MUL);  /* Multiply texture with color */
    } else {
        x_combine(X_COLOR);  /* Use vertex colors only */
    }
    
    /* Begin drawing quads */
    x_begin(X_QUADS);
    
    /* Draw each face of the cube */
    for (i = 0; i < 6; i++) {
        int* face = g_cube_faces[i];
        
        /* Vertex 0 */
        x_vxpos(g_cube_vertices[face[0]][0],
                g_cube_vertices[face[0]][1],
                g_cube_vertices[face[0]][2]);
        x_vxcolor4(g_cube_colors[face[0]][0],
                   g_cube_colors[face[0]][1],
                   g_cube_colors[face[0]][2],
                   g_cube_colors[face[0]][3]);
        x_vxtex(g_cube_texcoords[face[0]][0],
                g_cube_texcoords[face[0]][1]);
        
        /* Vertex 1 */
        x_vxpos(g_cube_vertices[face[1]][0],
                g_cube_vertices[face[1]][1],
                g_cube_vertices[face[1]][2]);
        x_vxcolor4(g_cube_colors[face[1]][0],
                   g_cube_colors[face[1]][1],
                   g_cube_colors[face[1]][2],
                   g_cube_colors[face[1]][3]);
        x_vxtex(g_cube_texcoords[face[1]][0],
                g_cube_texcoords[face[1]][1]);
        
        /* Vertex 2 */
        x_vxpos(g_cube_vertices[face[2]][0],
                g_cube_vertices[face[2]][1],
                g_cube_vertices[face[2]][2]);
        x_vxcolor4(g_cube_colors[face[2]][0],
                   g_cube_colors[face[2]][1],
                   g_cube_colors[face[2]][2],
                   g_cube_colors[face[2]][3]);
        x_vxtex(g_cube_texcoords[face[2]][0],
                g_cube_texcoords[face[2]][1]);
        
        /* Vertex 3 */
        x_vxpos(g_cube_vertices[face[3]][0],
                g_cube_vertices[face[3]][1],
                g_cube_vertices[face[3]][2]);
        x_vxcolor4(g_cube_colors[face[3]][0],
                   g_cube_colors[face[3]][1],
                   g_cube_colors[face[3]][2],
                   g_cube_colors[face[3]][3]);
        x_vxtex(g_cube_texcoords[face[3]][0],
                g_cube_texcoords[face[3]][1]);
    }
    
    /* End drawing */
    x_end();
    
    /* Flush any remaining geometry */
    x_flush();
}

/*
** Print statistics to the window
*/
void PrintStats(void)
{
    xt_stats stats;
    char buffer[256];
    
    /* Get statistics */
    x_getstats(&stats, sizeof(stats));
    
    /* Format statistics string */
    snprintf(buffer, sizeof(buffer),
        "Cube Demo - x.h Library\n"
        "Rotation: X=%.1f Y=%.1f Z=%.1f\n"
        "Position: X=%.2f Y=%.2f\n"
        "Vertices: %d  Triangles: %d\n"
        "Mode changes: %d  Texture changes: %d",
        g_cube_rotation_x, g_cube_rotation_y, g_cube_rotation_z,
        g_cube_position_x, g_cube_position_y,
        stats.in_vx, stats.in_tri,
        stats.chg_mode, stats.chg_text);
    
    /* Output to log file */
    x_log("%s\n", buffer);
    
    /* Display on window (if we had proper GDI drawing) */
    /* For now, just log it */
}

/*
** Window procedure
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_PAINT:
            /* Main rendering loop */
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd, &ps);
                
                /* Clear the screen */
                x_clear(1, 1, 0.1f, 0.1f, 0.2f);  /* Dark blue background */
                
                /* Update and draw the cube */
                UpdateCube();
                DrawCube();
                
                /* Finish frame (swap buffers) */
                x_finish();
                
                EndPaint(hwnd, &ps);
            }
            break;
            
        case WM_SIZE:
            /* Handle window resize */
            g_window_width = LOWORD(lParam);
            g_window_height = HIWORD(lParam);
            
            /* Resize the graphics context */
            x_resize(g_window_width, g_window_height);
            
            /* Reset viewport */
            x_viewport(0.0f, 0.0f, (float)g_window_width, (float)g_window_height);
            
            /* Reset projection */
            x_projection(45.0f, 0.9f, 65535.0f);
            break;
            
        case WM_KEYDOWN:
            /* Handle keyboard input */
            switch (wParam) {
                case VK_ESCAPE:
                    /* Exit on Escape */
                    PostQuitMessage(0);
                    break;
                    
                case VK_SPACE:
                    /* Toggle fullscreen on Space */
                    x_fullscreen(1);
                    break;
                    
                case 'W':
                    /* Toggle wireframe mode */
                    {
                        static int wireframe = 0;
                        wireframe = !wireframe;
                        if (wireframe) {
                            x_geometry(X_WIRE);
                        } else {
                            x_geometry(X_CULLBACK);
                        }
                    }
                    break;
                    
                case 'C':
                    /* Toggle culling */
                    {
                        static int cull = 1;
                        cull = !cull;
                        if (cull) {
                            x_geometry(X_CULLBACK);
                        } else {
                            x_geometry(0);
                        }
                    }
                    break;
                    
                case 'F':
                    /* Toggle fog */
                    {
                        static int fog = 1;
                        fog = !fog;
                        if (fog) {
                            x_fog(X_LINEAR, 1.0f, 20.0f, 0.0f, 0.0f, 0.0f);
                        } else {
                            x_fog(X_DISABLE, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                        }
                    }
                    break;
            }
            break;
            
        case WM_DESTROY:
            CleanupDemo();
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*
** Main entry point
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    BOOL result;
    
    /* Initialize the demo */
    if (!InitializeDemo(hInstance, nCmdShow)) {
        MessageBox(NULL, "Failed to initialize demo", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    /* Main message loop */
    while (1) {
        /* Check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            /* No messages - render a frame */
            /* Force a paint to keep animation running */
            InvalidateRect(g_hwnd, NULL, FALSE);
            UpdateWindow(g_hwnd);
            
            /* Small delay to control frame rate */
            x_sleep(16);  /* ~60 FPS */
        }
    }
    
    return (int)msg.wParam;
}
