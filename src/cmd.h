// Console commands processor
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Registers the console command handlers in the command table.
 */
void cmd_init();

/**
 * Executes a console command string, using ';' as a command separator.
 * @param cmd Command string to execute.
 */
// execute a command, ';' acts as a command separator for multiple commands
// quotes etc are not supported, so ';' inside a quote is still a separator!
void command(char* cmd);

#ifdef __cplusplus
};
#endif
