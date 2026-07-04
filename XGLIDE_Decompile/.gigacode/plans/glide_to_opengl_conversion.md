# Glide to OpenGL Conversion Plan

## Overview
This project converts a Glide wrapper library to use OpenGL instead. Glide was 3Dfx's legacy 3D graphics API used on Voodoo graphics cards. The goal is to maintain API compatibility while using modern OpenGL for rendering.

## Current Architecture Analysis

### Existing Components:
1. **x.h** - Public header defining the API
2. **api.c** - Main API implementation with Glide calls
3. **fx.c** - Mode setting and state management
4. **fxtext.c** - Texture memory management
5. **fxgeom.c** - Vertex transformation and clipping
6. **state.h** - State structures
7. **util.c** - Utility functions (memory, timing, logging)

### Key Glide API Calls Used:
- `grGlideInit()`, `grGlideShutdown()`
- `grSstSelect()`, `grSstWinOpen()`
- `grBufferSwap()`, `grBufferClear()`
- `grColorMask()`, `grDepthMask()`
- `grCullMode()`, `grDepthBufferMode()`, `grDitherMode()`
- `grTexCombine()`, `grTexFilterMode()`, `grTexClampMode()`
- `grAlphaBlendFunction()`, `grAlphaTestFunction()`
- `grFogTable()`, `grFogMode()`
- `grDrawTriangle()`, `grDrawLine()`, `grDrawPoint()`

## Conversion Plan

### Phase 1: Core OpenGL Wrapper (xgl.c/xgl.h)

#### 1.1 OpenGL Context Initialization
- Create `xgl_init()` function for OpenGL context setup
- Use Windows WGL for context creation
- Support double buffering, depth buffer, and vsync
- Handle window resizing with `xgl_resize()`

#### 1.2 State Management
- Keep existing state structures (`xt_state`, `xt_rendmode`)
- Replace Glide-specific state with OpenGL equivalents
- Maintain compatibility with existing `xmode.c` logic

#### 1.3 Drawing Functions
- Replace Glide draw calls with OpenGL:
  - `glDrawArrays()` / `glDrawElements()` instead of Glide primitives
  - Use vertex arrays or VBOs
  - Maintain vertex format compatibility

### Phase 2: Rendering Pipeline

#### 2.1 Color and Depth
- Replace `grColorMask()` with `glColorMask()`
- Replace `grDepthMask()` with `glDepthMask()`
- Replace `grDepthBufferMode()` with `glDepthFunc()` and `glDepthMask()`
- Replace `grDitherMode()` with `glEnable(GL_DITHER)`

#### 2.2 Texturing
- Replace Glide texture functions with OpenGL:
  - `glGenTextures()`, `glBindTexture()`
  - `glTexImage2D()` for texture upload
  - `glTexParameteri()` for filtering/clamping
  - Support multitexturing with `glActiveTexture()`

#### 2.3 Blending and Alpha
- Replace `grAlphaBlendFunction()` with `glBlendFunc()`
- Replace `grAlphaTestFunction()` with `glEnable(GL_ALPHA_TEST)` and `glAlphaFunc()`
- Support all Glide blend modes

#### 2.4 Fog
- Replace `grFogTable()` and `grFogMode()` with OpenGL fog
- Use `glFogfv()` and `glEnable(GL_FOG)`

#### 2.5 Culling
- Replace `grCullMode()` with `glEnable(GL_CULL_FACE)` and `glCullFace()`

### Phase 3: Geometry Processing

#### 3.1 Vertex Transformation
- Keep existing `xform()` function in `fxgeom.c`
- Use OpenGL matrix stack or shaders for projection
- Maintain clip handling

#### 3.2 Primitive Rendering
- Convert Glide primitives to OpenGL equivalents:
  - Points: `glDrawArrays(GL_POINTS, ...)`
  - Lines: `glDrawArrays(GL_LINES, ...)`
  - Triangles: `glDrawArrays(GL_TRIANGLES, ...)`
  - Strips and fans: `glDrawArrays(GL_TRIANGLE_STRIP, ...)`

### Phase 4: Texture Management

#### 4.1 Texture Upload
- Replace Glide texture download with OpenGL `glTexImage2D()`
- Maintain texture memory management (`fxtext.c`)
- Support all texture formats (RGBA5551, RGB565, ARGB4444, etc.)

#### 4.2 Texture States
- Replace Glide texture mode functions
- Support mipmap generation with `glGenerateMipmap()`
- Handle texture memory allocation/deallocation

### Phase 5: Framebuffer Operations

#### 5.1 Buffer Swapping
- Replace `grBufferSwap()` with `SwapBuffers()` or `glFinish()`

#### 5.2 Framebuffer Reading
- Replace `grLfbReadRegion()` with `glReadPixels()`

## Implementation Steps

### Step 1: Create xgl.h and xgl.c
- Define OpenGL-specific functions
- Include OpenGL headers
- Define function pointers for multitexturing extensions

### Step 2: Implement xgl_init() and context management
- WGL context creation
- Pixel format setup
- Display list and vertex array support

### Step 3: Rewrite fx.c for OpenGL
- Replace Glide calls with OpenGL equivalents
- Maintain mode state machine
- Keep compatibility with existing mode logic

### Step 4: Rewrite fxtext.c for OpenGL
- Replace Glide texture download with OpenGL texture upload
- Keep memory management logic
- Support all texture formats

### Step 5: Update fxgeom.c
- Keep vertex transformation logic
- Update primitive drawing for OpenGL
- Maintain clipping logic

### Step 6: Update api.c
- Replace Glide calls with OpenGL equivalents
- Maintain API compatibility

## Files to Create/Modify

### New Files:
- `xgl.h` - OpenGL header with API
- `xgl.c` - OpenGL implementation

### Modified Files:
- `api.c` - Replace Glide calls with OpenGL
- `fx.c` - Replace Glide mode functions
- `fxtext.c` - Replace Glide texture functions
- `fxgeom.c` - Update for OpenGL primitives

## Compatibility Goals

### Maintained:
- Same public API in x.h
- Same state management
- Same texture formats
- Same blending modes
- Same fog modes
- Same geometry processing

### Changed:
- Internal implementation uses OpenGL instead of Glide
- Context management uses WGL
- Texture upload uses OpenGL API
- Drawing uses OpenGL primitives

## Testing Strategy

1. Test basic rendering (points, lines, triangles)
2. Test texturing (single and multitexture)
3. Test blending modes
4. Test fog
5. Test depth testing
6. Test culling
7. Test framebuffer operations

## Notes

- Keep the existing state structures for compatibility
- Maintain the same vertex data structures
- Preserve the geometry processing logic
- Only change the low-level rendering calls
