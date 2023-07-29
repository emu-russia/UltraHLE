#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void boot(char* cartname, int nomemmap);

void reset(void);

#ifdef __cplusplus
};
#endif
