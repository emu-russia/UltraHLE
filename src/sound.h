#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes the DirectSound device and sets the cooperative level.
 * @param rate Requested sample rate (currently unused by the implementation).
 * @return 0 on success, -1 on DirectSound failure.
 */
int  sound_init(int rate);
/**
 * Creates a looping DirectSound buffer at the given sample rate and starts playback.
 * @param rate Sample rate in Hz.
 */
void sound_start(int rate);
/**
 * Stops playback and releases the DirectSound buffer.
 */
void sound_stop(void);

/**
 * Saves the current contents of the sound buffer to a file.
 * @param file Output file path.
 */
void sound_debugsavebuffer(char *file);

/**
 * Returns the number of bytes currently in the sound buffer waiting to be played.
 * @return Number of buffered bytes.
 */
int  sound_buffered(void); // returns bytes
/**
 * Returns the current playback position in the sound buffer and the buffer size.
 * @param bufsize Receives the total buffer size in bytes, may be NULL.
 * @return Current playback position in bytes.
 */
int  sound_position(int *bufsize); // returns bytes
/**
 * Adds samples to the sound buffer without checking for free space.
 * @param data Pointer to the 16-bit sample data.
 * @param bytes Number of bytes to add.
 * @return 0 on success, -2 when sound is not initialized, -1 on invalid byte count, 1 on buffer lock failure.
 */
int  sound_add(short *data,int bytes);
/**
 * Resets the buffer write position so that target bytes remain buffered, zeroing the newly exposed region.
 * @param target Desired number of buffered bytes.
 */
void sound_resync(int target);

/**
 * Appends samples to a WAV file, creating it with a header if needed and fixing the header sizes.
 * @param file Path of the WAV file.
 * @param data Sample data.
 * @param cnt Number of bytes to append.
 * @param stereo Non-zero for a stereo WAV header.
 */
void sound_addwavfile(char *file,short *data,int cnt,int stereo);

#ifdef __cplusplus
};
#endif
