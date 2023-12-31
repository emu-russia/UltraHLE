// Console commands processor
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void cmd_init();

// execute a command, ';' acts as a command separator for multiple commands
// quotes etc are not supported, so ';' inside a quote is still a separator!
void command(char* cmd);

#ifdef __cplusplus
};
#endif
