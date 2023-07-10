
int  sound_init(int rate);
void sound_start(int rate);
void sound_stop(void);

void sound_debugsavebuffer(char *file);

int  sound_buffered(void); // returns bytes
int  sound_position(int *bufsize); // returns bytes
int  sound_add(short *data,int bytes);
void sound_resync(int target);

void sound_addwavfile(char *file,short *data,int cnt,int stereo);
