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

#define KEY_A WPAD_BUTTON_A
#define KEY_B WPAD_BUTTON_B
#define KEY_X WPAD_BUTTON_1
#define KEY_Y WPAD_BUTTON_2
#define KEY_L WPAD_BUTTON_MINUS
#define KEY_R WPAD_BUTTON_PLUS
#define KEY_UP WPAD_BUTTON_UP
#define KEY_DOWN WPAD_BUTTON_DOWN
#define KEY_LEFT WPAD_BUTTON_LEFT
#define KEY_RIGHT WPAD_BUTTON_RIGHT
#define KEY_MENU WPAD_BUTTON_HOME

#elif defined(CC_BUILD_WIN)

#define KEY_A SDL_SCANCODE_A
#define KEY_B SDL_SCANCODE_B
#define KEY_X SDL_SCANCODE_X
#define KEY_Y SDL_SCANCODE_Y
#define KEY_L SDL_SCANCODE_L
#define KEY_R SDL_SCANCODE_R
#define KEY_UP SDL_SCANCODE_UP
#define KEY_DOWN SDL_SCANCODE_DOWN
#define KEY_LEFT SDL_SCANCODE_LEFT
#define KEY_RIGHT SDL_SCANCODE_RIGHT
#define KEY_MENU SDL_SCANCODE_ESCAPE

#else

#define KEY_A 0
#define KEY_B 0
#define KEY_X 0
#define KEY_Y 0
#define KEY_L 0
#define KEY_R 0
#define KEY_UP 0
#define KEY_DOWN 0
#define KEY_LEFT 0
#define KEY_RIGHT 0
#define KEY_MENU 0

#endif

int checkKey(int key);
int checkKeyHeld(int key);
Vector2 mousePos();