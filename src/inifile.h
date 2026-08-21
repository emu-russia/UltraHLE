#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Executes one ultra.ini command line (setting, path, patch, etc).
 * @param cmd Command line text to execute.
 */
void inifile_command(char* cmd);
/**
 * Reads global settings from ultra.ini and cart-specific settings for the given cart.
 * @param cartnamep Cart name used to select the ini sections.
 */
void inifile_read(char* cartnamep);
/**
 * Reads ultra.ini settings into a temporary state and restores the original cart data.
 * @param cartnamep Cart name used to select the ini sections.
 */
void inifile_readtemp(char* cartnamep);
/**
 * Applies ultra.ini patches scheduled for a DMA transfer.
 * @param dmanum DMA number of the transfer, 0 applies memory maps, -1 applies every frame.
 */
void inifile_patches(int dmanum);

#ifdef __cplusplus
};
#endif
