// Logging

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// There should be minimal stuff on the generic info channel, so that by
// enabling it only and running a game you can see if any problems
// (warnings etc) come up. Info is always enabled if any other channel
// is enabled.

// Some things (trace, ops) are printed with 'print' but have their
// separet st.dump-enables.

/**
 * Prints a formatted message to the console and the log file.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
// The following print to console (logged to ultra.log)
void print(const char* txt, ...);  // show always
/**
 * Prints a formatted message to the generic info channel when info dumping is enabled.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
void logi(char* txt, ...);   // generic stuff (toggle 'info')
/**
 * Prints a formatted message to the compiler channel when assembly dumping is enabled.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
void logc(char* txt, ...);   // compiler stuff
/**
 * Prints a formatted message to the hardware channel when hardware dumping is enabled.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
void logh(char* txt, ...);   // hardware stuff (toggle 'hw')
/**
 * Prints a formatted message to the operating system channel when OS dumping is enabled.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
void logo(char* txt, ...);   // operating system stuff (toggle 'os')

/**
 * Appends a formatted message to the audio log file (SLIST.LOG).
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
// These two dump to separate logfiles
void loga(char* txt, ...);   // audio log (SLIST.LOG)
/**
 * Appends a formatted message to the display log file (DLIST.LOG) when display dumping is enabled.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
void logd(char* txt, ...);   // display log (DLIST.LOG)

/**
 * Flushes all log files to disk.
 */
// flush all log files to disk
void flushlog(void);

// These are used to report problems, and can stop emulation at next
// convenient point (next instruction on sgo, next group on ago).

/**
 * Reports a fatal exception: always prints and stops emulation.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
// Exception always stops execution and is always printed
// Use this for breakpoints or for inserting testing-breaks to code
void exception(char* txt, ...);
/**
 * Reports an error, stopping emulation when st.stoperror is set.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
// Error acts like exception is st.stoperror=1, otherwise printed to info
void error(char* txt, ...);
/**
 * Reports a warning, stopping emulation when st.stopwarning is set.
 * @param txt Format string.
 * @param ... Additional arguments for the format string.
 */
// Warning acts like exception is st.stopwarning=1, otherwise printed to info
void warning(char* txt, ...);

// NOTE: print and log* don't include linefeeds (so use \n) but
// exception,error,warning do add a \n at end of string!

#ifdef __cplusplus
};
#endif
