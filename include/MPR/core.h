// Project MPRender Core header

#ifndef CORE_H
#define CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Windows
#ifdef CC_BUILD_WIN
    #include <windows.h>
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_ttf.h>
    #include <SDL2/SDL_mixer.h>
    #include <GL/glew.h>
    #include <GL/gl.h>
#endif

// Wii
#ifdef CC_BUILD_GCWII
    #include <grrlib.h>
    #include <ogcsys.h>
    #include <gccore.h>
    #include <wiiuse/wpad.h>
    #include <fat.h>
    #ifdef CC_BUILD_LIB_GCWII
    #include "font_regular_ttf.h"
    #endif
#endif

#ifndef f32
typedef float f32;
#endif

#ifndef u32
typedef unsigned int u32;
#endif


// Structures

typedef struct {
    unsigned char* data;
    uint64_t size;
} RawData;

typedef struct {
    #ifdef CC_BUILD_WIN
        HANDLE thread;
    #endif

    #ifdef CC_BUILD_GCWII
        lwp_t thread;
    #endif
} Thread;

typedef struct {
    // Windows
    #ifdef CC_BUILD_WIN
        SDL_Window *window;
        SDL_Renderer *renderer;
        int width;
        int height;
    #endif
    
    // Wii
    #ifdef CC_BUILD_GCWII
        int width;
        int height;
    #endif
} Window;

typedef struct {
    int x;
    int y;
} Vector2;

typedef struct {
    float x;
    float y;
} Vector2f;

typedef struct {
    int x;
    int y;
    int z;
} Vector3;

typedef struct {
    int w;
    int h;
} Size;

typedef struct {
    int r;
    int g;
    int b;
    int a;
} Color;

typedef struct {
    #ifdef CC_BUILD_WIN
        TTF_Font* font;
        char* file;
        int size;
    #endif
    #ifdef CC_BUILD_GCWII
        GRRLIB_ttfFont* font;
        char* file;
        int size;
    #endif
} Font;

typedef struct {
    #ifdef CC_BUILD_WIN
        SDL_Texture *texture;
    #endif

    #ifdef CC_BUILD_GCWII
        GRRLIB_texImg *texture;
    #endif
    
    Color color;
    Size size;
    Vector2 position;
    float rotation;
    bool rotateAroundCenter;
} Texture;

typedef struct
{
    Vector2 pnt;
    Size size;
    Color color;

    Vector2f velocity;    // New: Velocity of the rectangle
    Vector2f acceleration; // New: Acceleration of the rectangle
} Rect;

typedef struct {
    Vector2 position;
    Size size;
    int radius;
    bool filled;
    Color color;
} Circle;

typedef struct {
    Vector2 position;
    Size size;
    int radius;
    bool filled;
    Color color;
} Triangle;
typedef struct
{
    int length;
    Vector2 pnt;
    char* str;

    Color color;
    Font font;
} Text;
typedef struct {
    Vector2 pnt;
    Size size;
    Color color;
    Color textColor;
    int length;
    char* str;
    void (*func)(int, char**);
    Font font;
} Button;

typedef struct {
    bool show;
    Rect background;
    Rect foreground;
    Rect titleTextBackground;
    Text title;
    Text text;
    Button xButton;
    Button buttons[3];
    void (*func)(int, char**);
} MessageBoxRend;

typedef struct {
    Button button;
    bool checked;
} Checkbox;

typedef struct {
    f32 vertex[3];
    u32 color;
    Vector2 texturePos;
} Vertex;

typedef struct {
    int vertexCount;
    bool textured;
    char* textureFile;
    int textureSize;
    Vertex* vertex;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    #ifdef CC_BUILD_GCWII
        GRRLIB_texImg* texture;
    #endif
    #ifdef CC_BUILD_WIN
        GLuint texture;
    #endif
} Mesh;

typedef struct {
    int meshCount;
    Mesh* mesh;
} Model;

// our object struct
typedef struct {
    Size size;
    Vector2 pnt;
} Object;

#ifdef CC_BUILD_GCWII
#define wait(ms) usleep(ms * 1000)
#endif
#ifdef CC_BUILD_WIN
#define wait(ms) _sleep(ms)
#endif

extern Window win;

void testGifLib();

RawData loadRawData(char* filename);
Thread createThread(void* func, void* arg);
Window init();
void problem(Window win, char* txt, bool exiter);

void go3dMode(f32 minDistance, f32 maxDistance, f32 fov, bool textureMode);
void go2dMode();

Font initFont(char* file, int size, Window win);
Text initText(char* str, int x, int y, int length, Color color, Font font);
Text rePosText(Text cText, int x, int y);
Rect initRect(int x, int y, int w, int h, Color color);
Rect rePosRect(Rect cRect, int x, int y, int w, int h);
Button createButton(int x, int y, int w, int h, char* str, Color color, Color textColor, Font fnt, void* click);
Button rePosButton(Button cButton, int x, int y, int w, int h);
MessageBoxRend createMessageBox(int x, int y, int w, int h, char* title, char* text, Color color, Color textColor, Font fnt, void* click);
MessageBoxRend rePosMessageBox(MessageBoxRend cmb, int x, int y, int w, int h);
Texture initTexture(Window win, char* file, int x, int y, int w, int h, float rotation, bool rotateACent);
Texture loadTexture(Window win, unsigned char* data, int size, int x, int y, int w, int h, float rotation, bool rotateACent);

void renderTexture(Texture tex, Window win);
void renderText(Window window, Text text);
void renderRect(Window window, Rect rect);
void renderButton(Window window, Button button);
void renderMBox(Window window, MessageBoxRend msgBox, bool mouseDown, int mouseX, int mouseY);
Size getTextWidthTTF(Font font, Text text);

void simpleCollisionDetector(Object* sprite1, Object* sprite2, int *xHit, int *yHit);
void collisionDetector(Object* obj1, Object* obj2, int *xHit, int *yHit);
bool collisionDetectorBool(Object* obj1, Object* obj2);
void checkButtonClicked(Button button, bool mouseDown, int mouse_x, int mouse_y, int argc, char** argv);

void updateWindow(Window win);
void clearScreen(Window window);
void setRenderColor(Window window, Color color);
void fillScreen(Window win, Color color);

void renderMesh(Window win, Mesh mesh);
void renderModel(Window win, Model model);
void saveModel(Model model, char* file);
Model loadModel(char* file);

#endif