#ifndef __CTR_MEMORY_H__
#define __CTR_MEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif

int  _SetMemoryPermission(void *buffer, int size, int permission);
void _InvalidateInstructionCache(void);
void _FlushDataCache(void);
void _InvalidateAndFlushCaches(void);
int  _InitializeSvcHack(void);

#ifdef __cplusplus
}
#endif

#endif //__CTR_MEMORY_H__