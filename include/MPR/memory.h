// Memory Management utilities
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// DevKitPPC stuff
#ifdef CC_BUILD_GCWII
#include <gctypes.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <asndlib.h>
#include <ogcsys.h>
#endif

#ifdef CC_BUILD_WIN
#include <Windows.h>
// rendering for Windows is done with SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define u8 Uint8
#define u16 Uint16
#define u32 Uint32
#define s8 Sint8
#define s16 Sint16
#define s32 Sint32
#define f32 float
#define f64 double
#define s64 Sint64
#define u64 Uint64
#endif

// memManagement stuff
#define MB (1024 * 1024)

#ifdef CC_BUILD_GCWII

#define TOTAL_AVAILABLE_MEMORY 24 * MB // 24 MB

#endif
#ifdef CC_BUILD_WIN

#define TOTAL_AVAILABLE_MEMORY 512 * MB // 8 GB

#endif

char* bytes_to_KMGTP(u64 bytes);
void* allocateMemory(u64 size);
void* allocateClearedMemory(u64 arg1, u64 arg2);
void* allocateMemoryAlligned(u64 size, u64 alignment);
void freeMemory(void* ptr);
u64 getAvailableMemory();
u64 getUsedMemory();
void printUsage();