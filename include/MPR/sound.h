// MPRend - Controller input
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

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

#ifdef CC_BUILD_GCWII
#include <grrlib.h>
#include <fat.h>
#include <ogcsys.h>
#include <gccore.h>
#include <asndlib.h>
#include <wiiuse/wpad.h>
#endif

// define audio formats
#ifdef CC_BUILD_WIN

#define SOUND_S16SYS AUDIO_S16SYS
#define SOUND_U16SYS AUDIO_U16SYS
#define SOUND_S8 AUDIO_S8
#define SOUND_U8 AUDIO_U8
#define SOUND_F32SYS AUDIO_F32SYS

#endif
#ifdef CC_BUILD_GCWII

#define SOUND_S16SYS VOICE_STEREO_16BIT
#define SOUND_U16SYS VOICE_STEREO_16BIT
#define SOUND_S8 VOICE_MONO_8BIT
#define SOUND_U8 VOICE_MONO_8BIT
#define SOUND_F32SYS VOICE_STEREO_16BIT

#endif

#ifdef CC_BUILD_GCWII
s16 *leBuf_to_beBuf(s16 *buf, long bufSize);
void enqueueBuffer(void* buf, long bufSize);
void playNextAudioBuffer();
void initAudioQueue();
#endif

void beep(double f, int t);
void playBuffer(void* buf, long bufSize);
void playOgg(const void *buffer, s32 len, int time_pos, int mode);