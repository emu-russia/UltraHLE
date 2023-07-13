/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: /Releases/Glide2.1/devel/sst1/glide/src/gump.h 3     1/13/97 6:25p Jdt $
** $Log: /Releases/Glide2.1/devel/sst1/glide/src/gump.h $
 * 
 * 3     1/13/97 6:25p Jdt
 * 
 * 3     1/13/97 6:25p Jdt
**
*/

/* Multipass drawing */

#ifndef __GUMP_H__
#define __GUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define GLIDE_NUM_VIRTUAL_TMU   2   /* Number of virtual TMUs */

typedef FxU32 GrMPTextureCombineFnc_t;
#define GR_MPTEXTURECOMBINE_ADD           0x0 /*  */
#define GR_MPTEXTURECOMBINE_MULTIPLY      0x1 /*  */
#define GR_MPTEXTURECOMBINE_DETAIL0       0x2 /*  */
#define GR_MPTEXTURECOMBINE_DETAIL1       0x3 /*  */
#define GR_MPTEXTURECOMBINE_TRILINEAR0    0x4 /*  */
#define GR_MPTEXTURECOMBINE_TRILINEAR1    0x5 /*  */
#define GR_MPTEXTURECOMBINE_SUBTRACT      0x6 /*  */

typedef struct {
  GrMipMapId_t              mmid[GLIDE_NUM_VIRTUAL_TMU];
  GrMPTextureCombineFnc_t   tc_fnc;
} GrMPState;

FX_ENTRY void FX_CALL guMPInit( void );
FX_ENTRY void FX_CALL guMPTexCombineFunction( GrMPTextureCombineFnc_t tc );
FX_ENTRY void FX_CALL guMPTexSource( GrChipID_t virtual_tmu, GrMipMapId_t mmid );
FX_ENTRY void FX_CALL guMPDrawTriangle( const GrVertex *a, const GrVertex *b, const GrVertex *c );

#ifdef __cplusplus
}
#endif

#endif /* __GUMP_H__ */
