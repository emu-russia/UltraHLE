#include "pch.h"
#include "xgl.h"

// OpenGL geometry rendering implementation

#define MAX_VERTICES 256

// OpenGL vertex arrays
static float vertex_array[MAX_VERTICES * 3];
static float color_array[MAX_VERTICES * 4];
static float texcoord0_array[MAX_VERTICES * 2];
static float texcoord1_array[MAX_VERTICES * 2];

// Vertex count for current batch
static int g_vertex_count = 0;

// Current primitive type
static int g_primitive_type = 0;

// Vertex data buffer (from fxgeom.c)
extern xt_xfpos xfpos[0x200];
extern xt_pos pos[0x200];
extern xt_tex tex[0x200];
extern xt_tex tex2[0x200];
extern GrVertex grvx[0x200];
extern uint8_t xformed[0x200];
extern int vertices;
extern int vertices_base;
extern int corners_base;
extern int corners;
extern int corner[0x200 * 5];
extern int state;
extern int mode;

// Initialize geometry subsystem
void xgl_geom_init(void)
{
    g_vertex_count = 0;
    g_primitive_type = 0;
}

// Set up vertex arrays for OpenGL
static void setup_vertex_arrays(void)
{
    if (g_state[XST].send & 1) {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, 0, color_array);
    } else {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    if (g_state[XST].send & 2) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, texcoord0_array);
    } else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (g_state[XST].send & 4) {
        if (GLEW_ARB_multitexture) {
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, texcoord1_array);
            glActiveTextureARB(GL_TEXTURE0_ARB);
        }
    } else {
        if (GLEW_ARB_multitexture) {
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glActiveTextureARB(GL_TEXTURE0_ARB);
        }
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertex_array);
}

// Convert vertex data to OpenGL format
static void convert_vertex(int idx)
{
    // Convert position
    vertex_array[idx * 3] = xfpos[idx].x;
    vertex_array[idx * 3 + 1] = xfpos[idx].y;
    vertex_array[idx * 3 + 2] = xfpos[idx].z;

    // Convert color
    if (g_state[XST].send & 1) {
        color_array[idx * 4] = grvx[idx].r / 255.0f;
        color_array[idx * 4 + 1] = grvx[idx].g / 255.0f;
        color_array[idx * 4 + 2] = grvx[idx].b / 255.0f;
        color_array[idx * 4 + 3] = grvx[idx].a / 255.0f;
    }

    // Convert texture coordinates
    if (g_state[XST].send & 2) {
        texcoord0_array[idx * 2] = tex[idx].s;
        texcoord0_array[idx * 2 + 1] = tex[idx].t;
    }

    // Convert secondary texture coordinates
    if (g_state[XST].send & 4) {
        texcoord1_array[idx * 2] = tex2[idx].s;
        texcoord1_array[idx * 2 + 1] = tex2[idx].t;
    }
}

// Begin drawing with specified primitive type
void xgl_begin(int type)
{
    g_primitive_type = type;
    g_vertex_count = 0;

    // Flush any pending geometry
    if (vertices > 0) {
        xgl_flush();
    }
}

// End drawing
void xgl_end(void)
{
    if (g_primitive_type == X_POLYGON) {
        int poly_count = corners - corners_base - 1;
        if (poly_count >= 3) {
            corner[corners_base] = poly_count;
        } else {
            corners = corners_base;
            vertices = vertices_base;
        }
    }

    g_primitive_type = 0;

    if (vertices > 127) {
        xgl_flush();
    }
}

// Process vertex data
void xgl_vertexdata(xt_data* data)
{
    if (mode == 0) {
        x_fatal("vertex without begin");
    }

    // Process vertex data (same as original fxgeom.c)
    vertexdata(data);
}

// Transform vertices
void xgl_xform(xt_xfpos* dst, xt_pos* src, int count, char* mask)
{
    xform(dst, src, count, mask);
}

// Flush OpenGL geometry
void xgl_flush(void)
{
    if (!allxformed) {
        xform(xfpos, pos, vertices, xformed);
    }

    if (vertices == 0) {
        return;
    }

    // Setup vertex arrays
    setup_vertex_arrays();

    // Convert all vertices to OpenGL format
    for (int i = 0; i < vertices; i++) {
        convert_vertex(i);
    }

    // Draw geometry based on primitive type
    switch (g_primitive_type) {
        case X_POINTS:
            if (vertices > 0) {
                glDrawArrays(GL_POINTS, 0, vertices);
            }
            break;

        case X_LINES:
            if (vertices >= 2) {
                glDrawArrays(GL_LINES, 0, vertices);
            }
            break;

        case X_POLYLINE:
            if (vertices >= 2) {
                glDrawArrays(GL_LINE_STRIP, 0, vertices);
            }
            break;

        case X_TRIANGLES:
            if (vertices >= 3) {
                glDrawArrays(GL_TRIANGLES, 0, vertices);
            }
            break;

        case X_TRISTRIP:
            if (vertices >= 3) {
                glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices);
            }
            break;

        case X_TRIFAN:
            if (vertices >= 3) {
                glDrawArrays(GL_TRIANGLE_FAN, 0, vertices);
            }
            break;

        case X_QUADS:
            if (vertices >= 4) {
                glDrawArrays(GL_QUADS, 0, vertices);
            }
            break;

        case X_QUADSTRIP:
            if (vertices >= 4) {
                glDrawArrays(GL_QUAD_STRIP, 0, vertices);
            }
            break;

        case X_POLYGON:
            if (vertices >= 3) {
                glDrawArrays(GL_POLYGON, 0, vertices);
            }
            break;

        default:
            break;
    }

    // Disable vertex arrays
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if (GLEW_ARB_multitexture) {
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glActiveTextureARB(GL_TEXTURE0_ARB);
    }

    // Reset vertex counter
    vertices = 0;
    vertices_base = 0;
    corners = 0;
    corners_base = 0;
    state = 0;
    allxformed = 1;
}

// Add single vertex
void xgl_vx(xt_pos* pos, xt_data* data)
{
    x_vx(pos, data);
}

// Add vertex from array
void xgl_vxa(int arrayindex, xt_data* data)
{
    x_vxa(arrayindex, data);
}

// Add vertex array
void xgl_vxarray(xt_pos* pos, int size, char* mask)
{
    x_vxarray(pos, size, mask);
}
