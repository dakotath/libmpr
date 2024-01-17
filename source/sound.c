/**
 * @file sound.c
 * @details Sound functions.
 * @author Dakota Thorpe.
 * @date 2023-12-19
*/

// MPRend - Sound
#include <MPR/core.h>
#include <MPR/sound.h>
#include <MPR/memory.h>
#include <MPR/oggplayer.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>

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
#define MAX_QUEUE 10000
lwp_queue mainQueue;
#endif

#ifdef CC_BUILD_GCWII
/**
 * @brief Enqueues an audio buffer for playback on the Nintendo Wii.
 *
 * This function enqueues an audio buffer for playback on the Nintendo Wii using ASND.
 *
 * @param buf Pointer to the audio buffer.
 * @param bufSize Size of the audio buffer.
 * 
 * @return void
 */
void enqueueBuffer(void* buf, long bufSize, uint32_t freq, uint8_t channels) {
    // Play the audio buffer
    ASND_StopVoice(1);
    ASND_ChangeVolumeVoice(1, 128,128);
    ASND_SetVoice(1, VOICE_MONO_16BIT_LE, freq, 0, buf, bufSize, 255, 255, NULL);
    ASND_Pause(0);
}

/**
 * @brief Initializes the audio queue for Nintendo Wii.
 * 
 * This function initializes the audio queue for Nintendo Wii.
 * 
 * @return void
 */
void initAudioQueue() {
    // Create the audio thread
    //LWP_CreateThread(&audioThread, playNextAudioBuffer, NULL, NULL, 0, 64);
    //checkPlaying = createThread(cpFunc, NULL);
}

#endif

// test audio function
/**
 * @brief Generates a 1 kHz tone and plays it.
 *
 * This function generates a 1 kHz tone and plays it using either SDL2 (Windows) or ASND (Nintendo Wii).
 *
 * @return void
 */
void beep1khz()
{
    // make a 1khz tone for 1 second
    // 44100 srates, 16 bit, Stereo
    s16 buf[24000]; // 48000 / 2 = 24000
    int i;
    // short 16 bit limit is 65535
    int volume = 32767;
    int maxVol = 32767;
    for (i = 0; i < 24000; i++) {
        buf[i] = (s16)(sin(i * 1000 * M_PI / 48000) * volume);
        volume -= maxVol/24000;
        if(volume < 0) volume = 0;
    }

    // play the tone
    #ifdef CC_BUILD_WIN
    SDL_AudioSpec spec;
    spec.freq = 48000;
    spec.format = SOUND_S16SYS;
    spec.channels = 2;
    spec.samples = 24000;
    spec.callback = NULL;
    SDL_OpenAudio(&spec, NULL);
    SDL_QueueAudio(1, buf, 24000 * sizeof(s16));
    SDL_PauseAudio(0);
    #endif
    #ifdef CC_BUILD_GCWII
    //playerQueue = LWP_InitQueue(&mainQueue);
    //ASND_GetFirstUnusedVoice()
    ASND_SetVoice(ASND_GetFirstUnusedVoice(), SOUND_S16SYS, 48000, 0, buf, 24000, 255, 255, NULL);
    ASND_Pause(0);
    #endif
}

/**
 * @brief Generates a tone with a specified frequency and duration and plays it.
 *
 * This function generates a tone with a specified frequency and duration and plays it using either SDL2 (Windows) or ASND (Nintendo Wii).
 *
 * @param f The frequency of the tone.
 * @param t The duration of the tone.
 * 
 * @return void
 */
void beep(double f, int t)
{
    // make a 1khz tone for 1 second
    // 44100 srates, 16 bit, Stereo
    int SAMPLES = 48000/1000*t;
    s16 buf[SAMPLES]; // 48000 / 2 = 24000
    int i;
    // short 16 bit limit is 65535
    int volume = 32767;
    int maxVol = 32767;
    for (i = 0; i < SAMPLES; i++) {
        buf[i] = (s16)(sin(i * f * M_PI / 48000) * volume);
        volume -= maxVol/SAMPLES;
        if(volume < 0) volume = 0;
    }

    // play the tone
    #ifdef CC_BUILD_WIN
    SDL_AudioSpec spec;
    spec.freq = 48000;
    spec.format = SOUND_S16SYS;
    spec.channels = 2;
    spec.samples = SAMPLES;
    spec.callback = NULL;
    SDL_OpenAudio(&spec, NULL);
    SDL_QueueAudio(1, buf, SAMPLES * sizeof(s16));
    SDL_PauseAudio(0);
    #endif
    #ifdef CC_BUILD_GCWII
    //ASND_GetFirstUnusedVoice()
    ASND_SetVoice(ASND_GetFirstUnusedVoice(), SOUND_S16SYS, 48000, 0, buf, SAMPLES, 255, 255, NULL);
    ASND_Pause(0);
    #endif
}

/**
 * @brief Converts a little-endian audio buffer to a big-endian audio buffer.
 * 
 * This function converts a little-endian audio buffer to a big-endian audio buffer.
 * 
 * @param buf The little-endian audio buffer.
 * @param bufSize Size of the audio buffer.
 * 
 * @return A pointer to the newly allocated big-endian audio buffer.
 */
s16 *leBuf_to_beBuf(s16 *buf, long bufSize)
{
    s16 *newBuf = allocateMemory(bufSize);
    int i;
    for (i = 0; i < bufSize / 2; i++) {
        newBuf[i] = (buf[i] << 8) | ((buf[i] >> 8) & 0xFF);
    }
    return newBuf;
}

/**
 * @brief Plays an audio buffer.
 * 
 * This function plays an audio buffer using either SDL2 (Windows) or ASND (Nintendo Wii).
 * 
 * @param buf Pointer to the audio buffer.
 * @param bufSize Size of the audio buffer.
 * 
 * @return void
 */
void playBuffer(void* buf, long bufSize, uint32_t freq, uint8_t channels)
{
    #ifdef CC_BUILD_WIN
    // Pause the audio device to stop any currently playing audio
    SDL_PauseAudio(1);

    // Clear the audio buffer to remove any pending audio
    SDL_ClearQueuedAudio(1);
    SDL_AudioSpec spec;
    spec.freq = freq;
    spec.format = AUDIO_S16SYS;
    spec.channels = channels;
    spec.samples = bufSize / 4;
    spec.callback = NULL;
    SDL_OpenAudio(&spec, NULL);
    SDL_QueueAudio(1, buf, bufSize);
    SDL_PauseAudio(0);
    #endif
    #ifdef CC_BUILD_GCWII
    //ASND_GetFirstUnusedVoice()
    // since the Wii's PowerPC CPU is big endian, we need to convert the buffer to big endian
    //s16 *newBuf = leBuf_to_beBuf(buf, bufSize);
    //freeMemory(buf);
    enqueueBuffer(buf, bufSize, freq, channels);
    //ASND_SetVoice(1, VOICE_MONO_16BIT, 44100, 0, newBuf, bufSize, 255, 255, NULL);
    //ASND_Pause(0);
    #endif
}

#ifdef CC_BUILD_GCWII
/**
 * @brief Plays an Ogg audio stream.
 * 
 * This function plays an Ogg audio stream using either SDL2_mixer (Windows) or the built-in Ogg player (Nintendo Wii).
 * 
 * @param buffer Pointer to the Ogg audio stream buffer.
 * @param len Length of the Ogg audio stream.
 * @param time_pos Playback position in seconds (only applicable for SDL2_mixer).
 * @param mode Playback mode (only applicable for SDL2_mixer).
 * 
 * @return void
 */
void playOgg(const void *buffer, s32 len, int time_pos, int mode)
{
    PlayOgg(buffer, len, time_pos, mode);
}
#else
static Mix_Music* music = NULL;
/**
 * @brief Plays an Ogg audio stream.
 * 
 * This function plays an Ogg audio stream using either SDL2_mixer (Windows) or the built-in Ogg player (Nintendo Wii).
 * 
 * @param buffer Pointer to the Ogg audio stream buffer.
 * @param len Length of the Ogg audio stream.
 * @param time_pos Playback position in seconds (only applicable for SDL2_mixer).
 * @param mode Playback mode (only applicable for SDL2_mixer).
 * 
 * @return void
 */
void playOgg(const void *buffer, s32 len, int time_pos, int mode) {
    // Stop currently playing music, if any
    if (music) {
        Mix_FreeMusic(music);
        music = NULL;
    }

    // Initialize SDL2_mixer if not already initialized
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }

    // Load the music
    music = Mix_LoadMUS_RW(SDL_RWFromConstMem(buffer, len), 1);
    if (!music) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }

    // Set the music playback position if needed
    if (time_pos > 0) {
        Mix_RewindMusic();
        int skipSamples = (int)((double)time_pos * 44.1); // Assuming 44.1 kHz sample rate
        Mix_SetMusicPosition(skipSamples);
    }

    // Play the music
    Mix_PlayMusic(music, 1); // mode 1 means to loop the music

    // The music will continue playing in the background
}
#endif