#include "3dfx_headers/glide.h"

FX_ENTRY FxBool FX_CALL
grSstControl(FxU32 code)
{
	return FXTRUE;
}

FX_ENTRY FxBool FX_CALL
grSstQueryHardware(GrHwConfiguration* hwconfig)
{
	return FXTRUE;
}

FX_ENTRY void FX_CALL
grGlideShutdown(void)
{
}

FX_ENTRY void FX_CALL
grGlideInit(void)
{
}

FX_ENTRY void FX_CALL
grSstSelect(int which_sst)
{
}

FX_ENTRY FxBool FX_CALL
grSstWinOpen(
    FxU32                hWnd,
    GrScreenResolution_t screen_resolution,
    GrScreenRefresh_t    refresh_rate,
    GrColorFormat_t      color_format,
    GrOriginLocation_t   origin_location,
    int                  nColBuffers,
    int                  nAuxBuffers)
{
    return FXTRUE;
}

FX_ENTRY void FX_CALL
grClipWindow(int minx, int miny, int maxx, int maxy)
{
}

FX_ENTRY int FX_CALL
grBufferNumPending(void)
{
    return 0;
}

FX_ENTRY void FX_CALL
grBufferSwap(int swap_interval)
{
}

FX_ENTRY void FX_CALL
grColorMask(FxBool rgb, FxBool a)
{
}

FX_ENTRY void FX_CALL
grDepthMask(FxBool mask)
{
}

FX_ENTRY void FX_CALL
grBufferClear(GrColor_t color, GrAlpha_t alpha, FxU16 depth)
{
}

FX_ENTRY FxBool FX_CALL
grLfbReadRegion(GrBuffer_t src_buffer,
    FxU32 src_x, FxU32 src_y,
    FxU32 src_width, FxU32 src_height,
    FxU32 dst_stride, void* dst_data)
{
    return FXTRUE;
}

FX_ENTRY void FX_CALL
grDepthBiasLevel(FxI16 level)
{
}

FX_ENTRY void FX_CALL
grDepthBufferMode(GrDepthBufferMode_t mode)
{
}

FX_ENTRY void FX_CALL
grDitherMode(GrDitherMode_t mode)
{
}

FX_ENTRY void FX_CALL
grHints(GrHint_t hintType, FxU32 hintMask)
{
}

FX_ENTRY void FX_CALL
grTexFilterMode(
    GrChipID_t tmu,
    GrTextureFilterMode_t minfilter_mode,
    GrTextureFilterMode_t magfilter_mode
)
{
}

FX_ENTRY void FX_CALL
grTexMipMapMode(GrChipID_t     tmu,
    GrMipMapMode_t mode,
    FxBool         lodBlend)
{
}

FX_ENTRY void FX_CALL
grTexClampMode(
    GrChipID_t tmu,
    GrTextureClampMode_t s_clampmode,
    GrTextureClampMode_t t_clampmode
)
{
}

FX_ENTRY void FX_CALL
grTexCombine(
    GrChipID_t tmu,
    GrCombineFunction_t rgb_function,
    GrCombineFactor_t rgb_factor,
    GrCombineFunction_t alpha_function,
    GrCombineFactor_t alpha_factor,
    FxBool rgb_invert,
    FxBool alpha_invert
)
{
}

FX_ENTRY void FX_CALL
guFogGenerateLinear(
    GrFog_t fogtable[GR_FOG_TABLE_SIZE],
    float nearZ, float farZ)
{
}

FX_ENTRY void FX_CALL
guFogGenerateExp(GrFog_t fogtable[GR_FOG_TABLE_SIZE], float density)
{
}

FX_ENTRY void FX_CALL
grCullMode(GrCullMode_t mode)
{
}

FX_ENTRY void FX_CALL
grFogMode(GrFogMode_t mode)
{
}

FX_ENTRY void FX_CALL
grFogTable(const GrFog_t ft[GR_FOG_TABLE_SIZE])
{
}

FX_ENTRY void FX_CALL
grFogColorValue(GrColor_t fogcolor)
{
}

FX_ENTRY void FX_CALL
grDepthBufferFunction(GrCmpFnc_t function)
{
}

FX_ENTRY void FX_CALL
grConstantColorValue(GrColor_t value)
{
}

FX_ENTRY void FX_CALL
guColorCombineFunction(GrColorCombineFnc_t fnc)
{
}

FX_ENTRY void FX_CALL
guAlphaSource(GrAlphaSource_t mode)
{
}

FX_ENTRY void FX_CALL
grColorCombine(
    GrCombineFunction_t function, GrCombineFactor_t factor,
    GrCombineLocal_t local, GrCombineOther_t other,
    FxBool invert)
{
}

FX_ENTRY void FX_CALL
grAlphaTestReferenceValue(GrAlpha_t value)
{
}

FX_ENTRY void FX_CALL
grAlphaTestFunction(GrCmpFnc_t function)
{
}

FX_ENTRY void FX_CALL
grAlphaBlendFunction(
    GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df,
    GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df
)
{
}

FX_ENTRY void FX_CALL
grDrawLine(const GrVertex* v1, const GrVertex* v2)
{
}

FX_ENTRY void FX_CALL
grDrawPlanarPolygon(int nverts, const int ilist[], const GrVertex vlist[])
{
}

FX_ENTRY void FX_CALL
grDrawPoint(const GrVertex* pt)
{
}

FX_ENTRY void FX_CALL
grDrawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c)
{
}

FX_ENTRY FxU32 FX_CALL
grTexTextureMemRequired(FxU32     evenOdd,
    GrTexInfo* info)
{
    return 32;
}

FX_ENTRY void FX_CALL
grTexDownloadMipMap(GrChipID_t tmu,
    FxU32      startAddress,
    FxU32      evenOdd,
    GrTexInfo* info)
{
}

FX_ENTRY void FX_CALL
grTexSource(GrChipID_t tmu,
    FxU32      startAddress,
    FxU32      evenOdd,
    GrTexInfo* info)
{
}

FX_ENTRY FxU32 FX_CALL
grTexMinAddress(GrChipID_t tmu)
{
    return 0;
}

FX_ENTRY FxU32 FX_CALL
grTexMaxAddress(GrChipID_t tmu)
{
    return 16 * 1024 * 1024;
}
