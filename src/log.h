// Logging

#pragma once

// There should be minimal stuff on the generic info channel, so that by
// enabling it only and running a game you can see if any problems
// (warnings etc) come up. Info is always enabled if any other channel
// is enabled.

// Some things (trace, ops) are printed with 'print' but have their
// separet st.dump-enables.

// The following print to console (logged to ultra.log)
void print(char* txt, ...);  // show always
void logi(char* txt, ...);   // generic stuff (toggle 'info')
void logc(char* txt, ...);   // compiler stuff
void logh(char* txt, ...);   // hardware stuff (toggle 'hw')
void logo(char* txt, ...);   // operating system stuff (toggle 'os')

// These two dump to separate logfiles
void loga(char* txt, ...);   // audio log (SLIST.LOG)
void logd(char* txt, ...);   // display log (DLIST.LOG)

// flush all log files to disk
void flushlog(void);

// These are used to report problems, and can stop emulation at next
// convenient point (next instruction on sgo, next group on ago).

// Exception always stops execution and is always printed
// Use this for breakpoints or for inserting testing-breaks to code
void exception(char* txt, ...);
// Error acts like exception is st.stoperror=1, otherwise printed to info
void error(char* txt, ...);
// Warning acts like exception is st.stopwarning=1, otherwise printed to info
void warning(char* txt, ...);

// NOTE: print and log* don't include linefeeds (so use \n) but
// exception,error,warning do add a \n at end of string!
