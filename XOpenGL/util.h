
#pragma once

/**
 * Opens the log file in the given mode.
 * @param mode fopen mode string, or NULL to close the log file.
 */
void log_open(char* mode);
/**
 * Breaks execution in the debugger.
 */
void breakpoint();
