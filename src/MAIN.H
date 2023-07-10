// Public interface to the emulator

// ST: Communication with main emulator (main.c)
void  main_startup(void);
void  main_thread(void);

// works like printf for executing commands, returns all the text this
// command generates (up to 4K). Scan it for errors if you want.
//
// NOTE: If emulator is executing when command is given, main_stop is
// automatically called, and main_start executed after the command. If you
// want to do multiple commands while executing, it's better to first
// call main_stop manually, then the commands, and then restart.
char *main_command(char *cmd,...);

// this returns the same text returned by last main_commmand
char *main_commandreturn(void);
// this returns severity of errors from last main_command:
// 0=no errors, 1=warnings only, 2=errors, 3=exceptions
int   main_commanderrors(void);

// emulation go/stop (DO NOT USE command("go"))
int   main_stop(void); // returns 1 if unable to stop (emulator crashed)
int   main_start(void);
int   main_executing(void); // returns 1 if execution in progress

// deinitializes 3dfx. It is automatically reinitialized when executing
// and the rom generates a new frame. Execution no longer needs to be
// stopped when calling these.
void  main_hide3dfx(void);
void  main_show3dfx(void);

