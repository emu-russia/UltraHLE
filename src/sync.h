#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void sync_init(void);
void sync_thread(void);
void sync_gfxframedone(void);
void sync_retrace(void);
void sync_checkretrace(void);

#ifdef __cplusplus
};
#endif
