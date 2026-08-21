/** \file slist.h — Sound list execution. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Executes the sound list of an audio task.
 * @param task Audio task to execute.
 */
// SOUND - sound lists
void slist_execute(OSTask_t* task); // execute a sound list
/**
 * Queues the next audio buffer for playback.
 * @param m_addr Memory address of the audio data.
 * @param bytes Size of the audio data in bytes.
 * @return 0 on success, -1 if the audio output is busy.
 */
int  slist_nextbuffer(uint32_t m_addr, int bytes); // for playing audio (not implemented)
/**
 * Returns the current audio playback length.
 * @return Current audio length in samples, or 0 when idle.
 */
int  slist_getlength(void); // get current audio position (not implemented)

#ifdef __cplusplus
};
#endif
