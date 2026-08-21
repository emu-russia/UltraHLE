#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// SOUND - sound lists
void slist_execute(OSTask_t* task); // execute a sound list
int  slist_nextbuffer(uint32_t m_addr, int bytes); // for playing audio (not implemented)
int  slist_getlength(void); // get current audio position (not implemented)

#ifdef __cplusplus
};
#endif
