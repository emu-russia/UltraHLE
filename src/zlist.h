#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Decompresses a Zelda JPEG-DCT task into its picture buffer.
 * @param task Task containing the compressed picture and quantizer data.
 */
// ZELDA specific lists
void zlist_uncompress(OSTask_t* task);

#ifdef __cplusplus
};
#endif
