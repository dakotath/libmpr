/**
 * @file input.c
 * @details User input functions.
 * @author Dakota Thorpe.
 * @date 2023-12-19
*/

// MPRend - Controller input
#include <MPR/core.h>
#include <MPR/input.h>

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
#endif

#ifdef CC_BUILD_GCWII
#include <grrlib.h>
#include <fat.h>
#include <ogcsys.h>
#include <gccore.h>
#include <asndlib.h>
#include <wiiuse/wpad.h>
#endif

#ifdef CC_BUILD_GCWII
ir_t ir;
#endif

int checkKey(int key)
{
    #ifdef CC_BUILD_WIN

    // make an sdl event
    SDL_Event event;

    // poll the event
    SDL_PollEvent(&event);

    // get the key state
    const uint8_t* state = SDL_GetKeyboardState(NULL);
    if (state[key]) {
        return 1;
    }
    else {
        return 0;
    }
    #endif

    #ifdef CC_BUILD_GCWII
    WPAD_ScanPads();
    int i;
    for (i = 0; i < 4; i++) {
        s16 pressed = WPAD_ButtonsDown(i);
        if (pressed & key) {
            return 1;
        }
    }
    return 0;
    #endif
}

int checkKeyHeld(int key)
{
    #ifdef CC_BUILD_WIN

    // make an sdl event
    SDL_Event event;

    // poll the event
    SDL_PollEvent(&event);

    // get the key state
    const uint8_t* state = SDL_GetKeyboardState(NULL);
    if (state[key]) {
        return 1;
    }
    else {
        return 0;
    }
    #endif

    #ifdef CC_BUILD_GCWII
    WPAD_ScanPads();
    int i;
    for (i = 0; i < 4; i++) {
        s16 pressed = WPAD_ButtonsHeld(i);
        if (pressed & key) {
            return 1;
        }
    }
    return 0;
    #endif
}

Vector2 mousePos()
{
    #ifdef CC_BUILD_WIN
    int x, y;
    SDL_GetMouseState(&x, &y);
    Vector2 pos;
    pos.x = x;
    pos.y = y;
    return pos;
    #endif

    #ifdef CC_BUILD_GCWII
    Vector2 pos;
    WPAD_IR(WPAD_CHAN_0, &ir);
    pos.x = ir.x;
    pos.y = ir.y;
    return pos;
    #endif
}