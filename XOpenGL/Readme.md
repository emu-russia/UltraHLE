# XOpenGL — the X library ported to OpenGL

This folder contains a port of the **X library** (the graphics library used by
UltraHLE) from the Glide API to OpenGL. It sits next to
[`XGLIDE_Decompile`](../XGLIDE_Decompile), which is the decompiled original that
uses the Glide API as its backend.

The public API (`x.h`) is unchanged — the library is a drop-in replacement for
the emulator: it builds the same static library and exports the same X
functions that `rdp.c` uses.

## Layout

| File            | Purpose |
|-----------------|---------|
| `x.h`           | Public X API (identical to `XGLIDE_Decompile/x.h`) |
| `api.c`         | X API implementation — unchanged from the Glide version (it is backend-independent) |
| `state.h`       | `xt_state` / `xt_rendmode` (same layout) |
| `fx.c`          | OpenGL backend: display init (`init_*`) and pixel pipeline modes (`mode_*`) |
| `fxgeom.c`      | Geometry pipeline: vertex transform + clipping (same math as Glide) and the OpenGL drawing |
| `fxtext.c`      | Texture management: host-side RGBA8888 storage + OpenGL texture objects |
| `glcompat.c/.h` | OpenGL extension loading (multitexture, env combiners, blend-separate, fog coord) |
| `util.c`, `version.c` | Utilities / version (same as the Glide version) |
| `xdemo.c`       | Standalone demo/test program |

## How the port works

The original library did all its geometry transformation and clipping itself and
only asked Glide to rasterize triangles/lines/points with a fixed set of texture
combiners and blend factors. The port keeps the same structure and replaces the
Glide calls:

| Glide | OpenGL |
|-------|--------|
| `grSstWinOpen`, `grGlideInit` | WGL context creation on the given window |
| `grBufferSwap` | `SwapBuffers` |
| `grBufferClear`, `grClipWindow` | `glClear` + `glScissor` |
| `grLfbReadRegion` | `glReadPixels` |
| `grDrawTriangle/Line/Point` | immediate-mode `glBegin`/`glEnd` |
| `grTexDownloadMipMap`, `grTexSource` | `glTexImage2D` on per-handle texture objects |
| `grTexCombine`, `guColorCombineFunction` | `GL_ARB_texture_env_combine` per texture unit |
| `grAlphaBlendFunction` | `GL_EXT_blend_func_separate` |
| `grFogTable`, `guFogGenerate*` | `GL_EXT_fog_coord` with `GL_LINEAR`/`GL_EXP` |
| `grCullMode` | `glCullFace` |
| `grDepthBufferFunction` | `glDepthFunc` (W-buffer convention, see below) |

## Notable decisions / differences

* **Depth.** Glide used a W-buffer where the depth value was `oow = 1/w`
  (larger = closer). OpenGL has no W-buffer, so the port passes `oow` as the
  vertex Z. The ortho setup maps it to `window depth = (1-oow)/2`, so nearer
  fragments have smaller depth — the standard `GL_LESS` convention with the
  buffer cleared to 1.0. `X_ENABLE` enables the depth test (the current
  decompiled Glide backend maps it to a questionable `GR_CMP_EQUAL`).

* **Textures.** The original converted RGBA8888 data to packed 16-bit TMU
  formats to save Voodoo memory. OpenGL manages texture memory, so the port
  keeps RGBA8888 on the host and uploads it directly. `x_opentexturedata()`
  returns the same host buffer as before. The emulator provides the texture
  with row 0 = top of the image and maps the vertex coordinate t=0 to the top
  of the screen (the N64 convention); OpenGL samples v=0 from the first data
  row, so the rows are uploaded as-is - no vertical flip is needed.

* **Texture coordinates.** The vertex pipeline computes texture coordinates in
  the library's "256-unit" space (`texel * xmul`). The port normalizes them to
  OpenGL's `[0..1]` with the texture matrix (`1/xmul, 1/ymul`), set when a
  texture is selected. Perspective correction is preserved by passing the
  coordinates as `glTexCoord4f(sow, tow, 0, oow)` (s/t over w). The texture
  matrix scale equals the vertex coordinate scale, so the second texture unit
  of a dual-texture pass uses the first texture's scale (same as the original
  Glide backend).

* **Combiners.** The X combine modes map onto `GL_ARB_texture_env_combine`
  (`GL_MODULATE`, `GL_ADD`, `GL_SUBTRACT`, `GL_INTERPOLATE`, `GL_REPLACE`).
  `X_TEXTUREENVC`/`X_TEXTUREENVCR` use the texture *alpha* as the blend factor
  (GL cannot use the texture color as a factor); visually equivalent for
  most content. `X_MULADD` (marked "temp!" in the API) is approximated with
  `GL_MODULATE` — the emulator itself substitutes `X_MUL` for it.

* **Polygons.** The decompiled Glide backend cannot draw polygons with more
  than 3 vertices (`grDrawPlanarPolygon` was left commented out), so quads were
  dropped there. The OpenGL port draws them with a `GL_TRIANGLE_FAN`.

* **Fog.** The Glide fog table is replaced by `GL_LINEAR`/`GL_EXP` fog applied
  per-vertex through `glFogCoord` (fog coordinate = w). This produces exactly
  the fog curve the X API documents (min = 0% fog, max = 100%/90% fog).
  `X_LINEARADD` (additive fog, used in the second pass of two-cycle rendering)
  is approximated with linear fog.

* **Multitexture.** Dual-texture passes use `GL_ARB_multitexture`. If it is
  unavailable the library reports one TMU and the emulator falls back to
  multi-pass rendering.

* **x64 fixes.** The decompiled Glide backend contains 32-bit-era pointer
  truncation bugs (vertex-index lists passed through `int`, the `splitpoly`
  polygon fan, the clipped-polygon path) that break the x64 build; the port
  fixes all of them. The port is verified on both x86 and x64.

## Building

The Visual Studio project `Scripts/XOpenGL.vcxproj` builds the static library
`XOpenGL.lib` (same toolset v145 as the rest of the repo). It links only
`opengl32.lib`.

The emulator solution references this project instead of the Glide one; see
`Scripts/UltraHLE.sln`. To go back to the Glide backend, point the
`ProjectReference` in `Scripts/UltraHLE.vcxproj` back to
`XGLIDE_Decompile/Scripts/XGLIDE.vcxproj` and remove `opengl32.lib` from the
link line.

## Testing

`xdemo.c` is a small Win32 program that exercises the library: textured
perspective cube (z-buffering, gouraud shading) and an alpha-blended textured
2D overlay. Build it with `Scripts/xdemo.vcxproj` and run it — the window
should show a rotating checkerboard cube and a smooth gradient quad.
The same rendering path is used by the emulator (`rdp.c`), which builds and
links against `XOpenGL.lib`.
