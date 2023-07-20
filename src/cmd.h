#pragma once

// execute a command, ';' acts as a command separator for multiple commands
// quotes etc are not supported, so ';' inside a quote is still a separator!
void command(char* cmd);
