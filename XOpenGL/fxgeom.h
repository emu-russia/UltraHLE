
/** \file fxgeom.h — Vertex transformation, clipping and primitive setup. */

#pragma once

#define XFORM_MODE_FRUSTUM 0
#define XFORM_MODE_ORTHO 1
#define XFORM_MODE_PROJECT 2
#define XFORM_MODE_NONE 3

/**
 * Initializes the geometry subsystem.
 */
void geom_init();
/**
 * Logs a 4x4 matrix with the given name.
 * @param f Pointer to the matrix data.
 * @param name Name printed with the matrix.
 */
void dumpmatrix(float* f, char* name);
/**
 * Recomputes the projection matrix and viewport clip rectangle.
 */
void recalc_projection();
/**
 * Notifies the API layer that the projection matrix was recalculated.
 */
void projrecalced();	// defined in api.c
/**
 * Stores vertex attributes and appends the vertex to the current primitive.
 * @param data Vertex attribute data.
 */
void vertexdata(xt_data* data);
/**
 * Transforms vertex positions through the current combined matrix and
 * computes per-vertex clip flags.
 * @param dst Destination transformed vertices.
 * @param src Source vertex positions.
 * @param count Number of vertices.
 * @param mask Optional per-vertex flags; a zero entry requests a full transform,
 *             non-zero entries mark already-transformed or relative vertices.
 */
void xform(xt_xfpos* dst, xt_pos* src, int count, char* mask);
/**
 * Builds screen-space vertex data (position, depth, texture coordinates).
 * @param first Index of the first vertex.
 * @param count Number of vertices.
 * @return 0 when the vertices were set up.
 */
int setuprvx(int first, int count);
/**
 * Adds a vertex whose position is relative to the last absolute vertex.
 * @param p Vertex position.
 * @param d Vertex attribute data.
 */
void x_vxrel(xt_pos* p, xt_data* d);
/**
 * Clears the current primitive's vertex and corner data.
 */
void clear();
/**
 * Interpolates a new vertex where an edge crosses the given clip plane.
 * @param bit Clip plane flag to clip against.
 * @param vi Index of the first edge vertex.
 * @param vo Index of the second edge vertex.
 * @return Index of the new clipped vertex.
 */
int doclipvertex(int bit, int vi, int vo);
/**
 * Clips the current vertex list against one clip plane.
 * @param bit Clip plane flag to clip against.
 */
void doclip(int bit);
/**
 * Finalizes clipped vertices and returns the number of vertices in the clipped list.
 * @param vx Clipped vertex index list.
 * @return Number of vertices in the clipped list, or 0 if fully clipped.
 */
int clipfinish(int* vx);
/**
 * Clips a polygon against all six clip planes.
 * @param clor Combined clip flags of the polygon.
 * @param vxn Number of polygon vertices.
 * @param v Polygon vertex index list.
 * @param out Receives the clipped vertex index list.
 * @return Number of vertices in the clipped polygon, or 0 if fully clipped.
 */
int clippoly(int clor, int vxn, int* v, int** out);
/**
 * Clips one line endpoint against all six clip planes.
 * @param v1 Index of the endpoint to clip.
 * @param v2 Index of the other endpoint.
 * @return Index of the clipped endpoint.
 */
int docliplineend(int v1, int v2);
/**
 * Clips a line against all six clip planes.
 * @param v1 Index of the first endpoint.
 * @param v2 Index of the second endpoint.
 * @param out Receives the clipped endpoint index list.
 * @return Number of vertices in the clipped line, or 0 if fully clipped.
 */
int clipline(int v1, int v2, int** out);
/**
 * Triangulates a convex polygon into a triangle fan.
 * @param a1 Number of polygon vertices.
 * @param a2 Polygon vertex index list.
 * @param a3 Receives the triangle fan indices.
 * @return Number of indices written.
 */
int splitpoly(int a1, int* a2, DWORD* a3);
/**
 * Keeps the current primitive's tail vertices for continuation across flushes.
 */
void flush_reordertables();
/**
 * Draws all buffered primitives with OpenGL immediate-mode calls.
 * @return 0 on success.
 */
int flush_drawfx();
