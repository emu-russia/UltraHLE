#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void inifile_command(char* cmd);
void inifile_read(char* cartnamep);
void inifile_readtemp(char* cartnamep);
void inifile_patches(int dmanum);

#ifdef __cplusplus
};
#endif
