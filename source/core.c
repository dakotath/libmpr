/**
 * @file core.c
 * @details The meat and potatoes of MPRenderer.
 * @author Dakota Thorpe.
 * @date 2023-12-19
*/

// MPRend - Main function for rendering
#include <MPR/core.h>
#include <MPR/input.h>
#include <MPR/versions.h>
#include <MPR/memory.h>
#include <MPR/gifdec.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef CC_BUILD_GCWII

#include <grrlib.h>
#include <gctypes.h>

#endif

#ifdef CC_BUILD_GCWII
#include <grrlib.h>
#include <fat.h>
#include <ogcsys.h>
#include <gccore.h>
#include <asndlib.h>
#include <wiiuse/wpad.h>

extern ir_t ir;

#endif

#ifdef CC_BUILD_GCWII
GXRModeObj* rmode2 = NULL;
void* xfb2 = NULL;
#endif

/**
 * @brief Enter fallback console mode (works on wii).
*/
void enterBACKUP()
{
    #ifdef CC_BUILD_GCWII
    // deinit grrlib
    GRRLIB_Exit();

    // init the console (using console_init() from libogc)
	rmode2 = VIDEO_GetPreferredMode(NULL);
	xfb2 = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode2));
	console_init(xfb2,20,20,rmode2->fbWidth,rmode2->xfbHeight,rmode2->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode2);
	VIDEO_SetNextFramebuffer(xfb2);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
    #endif
}

/**
 * @brief Displays an error message in a window and optionally exits the program.
 *
 * This function displays an error message in a specified window using the given text.
 * <br>
 * It also provides an option to exit the program when the user presses the MENU key.
 *
 * @param win The window in which the error message will be displayed.
 * @param txt The error message text to be displayed.
 * @param exiter If true, the program will exit when the user presses the MENU key; otherwise, it continues running.
 *
 * @return void
 */
void problem(Window win, char* txt, bool exiter)
{
    Font font = initFont("assets/font_regular.ttf", 20, win);

    // load text
    char* errorText1 = malloc(100);
    char* errorText2 = malloc(100);
    int et1Len;
    int et2Len;

    sprintf(errorText1, "F*ck, theres a problem: %s.", txt);
    sprintf(errorText2, "Press KEY_HOME to exit.");

    et1Len = strlen(errorText1);
    et2Len = strlen(errorText2);

    Text Text1 = initText(errorText1, 0,0, et1Len, (Color){255,255,255,255}, font);
    Text Text2 = initText(errorText2, 0,font.size+1, et2Len, (Color){255,255,255,255}, font);

    while(!checkKey(KEY_MENU))
    {
        fillScreen(win, (Color){0x88,0x44,0x44,0xff});
        renderText(win, Text1);
        renderText(win, Text2);
        updateWindow(win);
    }

    if(exiter)
        exit(-1);
}

void go3dMode(f32 minDistance, f32 maxDistance, f32 fov, bool textureMode)
{
    #ifdef CC_BUILD_GCWII
        GRRLIB_3dMode(minDistance, maxDistance, fov, textureMode, 0);
    #endif
}
void go2dMode()
{
    #ifdef CC_BUILD_GCWII
        GRRLIB_2dMode();
    #endif
}

Window win;

#ifdef CC_BUILD_WIN
Thread winResizeThread;
static void* wrtFunc(void* arg)
{
    while(1)
    {
        wait(1);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                SDL_GetWindowSize(win.window, &win.width, &win.height);
                printf("Resized window size: %d x %d\n", win.width, win.height);
            }
        }
        //printf("%d\n", win.width);
    }
    return NULL;
}
#endif

/**
 * @brief Shows the splash screen.
 * 
 * This function is called in init() to show you the version of mprenderer.
 * <br>
 * We strongly recommend this not be disabled so that you know if the renderer is working at all.
 * 
 * @param win The window Parameter.
 * 
 * @return void
*/
void showSplash(Window win)
{
    //testGifLib(win);
    Font font = initFont("assets/font_regular.ttf", 20, win);
    Texture splashBg = initTexture(win, "assets/splash_bg.png", 0,0, win.width,win.height, 0, false);

    char* splashText = malloc(100);
    sprintf(splashText, "%s", CURRENT_VERSION);
    Text text1 = initText(splashText, 0,0, 0, (Color){0xff,0xff,0xff,0xff}, font);
    Size text1Size = getTextWidthTTF(font, text1);
    text1.pnt.x = (win.width - text1Size.w) / 2;
    text1.pnt.y = (win.height - text1Size.h) / 2;

    // fade in
    Color fadeColor;
    for (int i = 0; i <= 255; i += 2) {
        fadeColor = (Color){0xff, 0xff, 0xff, i};
        splashBg.color = fadeColor;
        text1.color = fadeColor;

        renderTexture(splashBg, win);
        renderText(win, text1);
        updateWindow(win);
    }

    wait(1000);

    // fade out
    for (int i = 255; i >= 0; i -= 2) {
        fadeColor = (Color){0xff, 0xff, 0xff, i};
        splashBg.color = fadeColor;
        text1.color = fadeColor;

        renderTexture(splashBg, win);
        renderText(win, text1);
        updateWindow(win);
    }

    free(splashText);
}

/**
 * @brief Startup the rendering engine.
 * 
 * Here is where everything that the engine needs get initialized.
 * <br>
 * When porting to other systems, make sure that you put the appropriate init's here.
 * 
 * @return Window
*/
Window init() {
    // window init, init SDL2
    printf("MPRend Version: '%s'\n", CURRENT_VERSION);
    #ifdef CC_BUILD_WIN
    //testGl();
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    TTF_Init();
    win.width = 640;
    win.height = 480;
    /*
    win.window = SDL_CreateWindow("GAME",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    win.width, win.height, 0);
    */
    SDL_Init(SDL_INIT_EVERYTHING);

    win.window = SDL_CreateWindow(
        "Spinning Cube with SDL2 and OpenGL",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        win.width, win.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    win.renderer = SDL_CreateRenderer(win.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_GLContext glContext = SDL_GL_CreateContext(win.window);
    if (!glContext) {
        printf("SDL_GL_CreateContext Error: %s\n", SDL_GetError());
    }

    // Initialize GLEW
    //glewExperimental = GL_TRUE;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    createThread(wrtFunc, NULL);

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("GLEW Error: %s\n", glewGetErrorString(glewError));
    }
    showSplash(win);
    //testGifLib(win);
    return win;
    #endif
    
    // Wii init, init GRRLIB, fat, and all that
    #ifdef CC_BUILD_GCWII
    // init video
    VIDEO_Init();

    // init grrlib
    GRRLIB_Init();
    GRRLIB_Camera3dSettings(0.0f,0.0f,100.0f, 0,1,0, 0,5.0f,0);
    // init fat
    if (!fatInitDefault()) {
        enterBACKUP();
        printf("error initializing FAT\n");
        while(1)
        {
            s16 pressed = WPAD_ButtonsDown(0);
            if(pressed & WPAD_BUTTON_HOME) exit(-1);
        }
    }

    // init wiimote
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_ALL, 640, 480);
    WPAD_IR(WPAD_CHAN_0, &ir);

    // init audio
    ASND_Init();

    initAudioQueue();

    Window win;
    win.width = 640;
    win.height = 480;
    showSplash(win);
    //testGifLib(win);
    return win;
    #endif
}

// Function to load file into RawData.
/**
 * @brief Load file into raw buffer.
 * 
 * When dealing with stuff that needs to read from buffers, use this to get files into buffers fast!
 * <br>
 * When cleaning up, don't forget to do a freeMemory() on the data for the buffer.
 * 
 * @param filename The file you wish to put into a buffer.
 * 
 * @return RawData
*/
RawData loadRawData(char* filename)
{
    RawData retData;

    FILE* rawFile = fopen(filename, "rb");
    if(!rawFile)
    {
        char* errtxt = allocateMemory(100);
        sprintf(errtxt, "Cannot Open: %s", filename);
        problem(win, errtxt, true);
    }

    fseek(rawFile, 0, SEEK_END);
    retData.size = ftell(rawFile);
    fseek(rawFile, 0, SEEK_SET);

    retData.data = allocateMemory(retData.size);
    fread(retData.data, retData.size, 1, rawFile);

    fclose(rawFile);

    return retData;
}

// create thread function
int curthread=0;
/**
 * @brief Create a thread so you can cook while you cook.
 * 
 * When you want to do some multithreading, create threads with this to do it.
 * <br>
 * Don't forget to wait() for at least 1ms within thread loops, or you could get a thread that does not allow for others to execute.
 * 
 * @param func Function for the thread.
 * @param arg The void* arguments. It's OK for it to be NULL.
 * 
 * @return Thread
*/
Thread createThread(void* func, void* arg)
{
    Thread thread;
    #ifdef CC_BUILD_WIN
    thread.thread = CreateThread(NULL, 0, func, arg, 0, NULL);
    #endif

    #ifdef CC_BUILD_GCWII
    LWP_CreateThread(&thread.thread, func, arg, NULL, 0, 80+curthread);
    if(thread.thread == LWP_THREAD_NULL)
    {
        enterBACKUP();
        printf("error initializing thread\n");
        while(1)
        {
            s16 pressed = WPAD_ButtonsDown(0);
            if(pressed & WPAD_BUTTON_HOME) exit(-1);
        }
    }
    curthread++;
    #endif
    return thread;
}

// SDL Specific functions

#ifdef CC_BUILD_WIN
SDL_Color convertColor(Color color)
{
    SDL_Color ret = {color.r, color.g, color.b, color.a};
    return ret;
}
#endif
#ifdef CC_BUILD_GCWII
u32 convertColor(Color color)
{
    u32 ret = (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
    return ret;
}
#endif

/**
 * @brief Init a TrueType Font.
 * 
 * If you like drawing text, then you need a font. This function can load a truetype font into the Font structure, so that you can render text.
 * <br>
 * Note: On font size's below 20, things go weird for some reason.
 * 
 * @param file TrueType Font File Location.
 * @param size The size of your font.
 * @param win Your window.
 * 
 * @return Font
*/
Font initFont(char* file, int size, Window win)
{
    #ifdef CC_BUILD_GCWII
        GRRLIB_ttfFont* font = GRRLIB_LoadTTF(font_regular_ttf, font_regular_ttf_size);
    #endif
    #ifdef CC_BUILD_WIN
        // implement sdl2 font load
        TTF_Font* font = TTF_OpenFont(file, size);
    #endif
    
    /*
    if (font == NULL)
    {
        char* errorText = malloc(100);
        #ifdef CC_BUILD_WIN
            memset(errorText, 0, 100);
            sprintf(errorText, "TTF_OpenFont Error: %s.", SDL_GetError());
        #endif
        #ifdef CC_BUILD_GCWII
            memset(errorText, 0, 100);
            sprintf(errorText, "TTF_OpenFont Error: %s.", "Just take a guess");
        #endif
        problem(win, errorText, true);
    }
    */
    Font ret;
    ret.font = font;
    ret.size = size;
    return ret;
}

/**
 * @brief Initializes a Text structure with the specified parameters.
 * 
 * This function creates a Text structure and initializes its attributes,
 * including the input string, position, length, color, and font.
 * The resulting Text structure can be used for rendering text in a window.
 * 
 * @param str The input string to be displayed.
 * @param x The x-coordinate of the text position.
 * @param y The y-coordinate of the text position.
 * @param length The length of the input string.
 * @param color The color of the text.
 * @param font The font to be used for rendering the text.
 * 
 * @return A Text structure initialized with the specified parameters.
 */
Text initText(char* str, int x, int y, int length, Color color, Font font)
{
    Text text;
    int strlength = strlen(str);
    
    // Allocate memory for the string and copy it
    text.str = malloc(strlength);
    strcpy(text.str, str);
    
    // Set the position, length, color, and font
    text.pnt.x = x;
    text.pnt.y = y;
    text.length = strlength;
    text.color = color;
    text.font = font;

    return text;
}

/**
 * @brief Repositions a Text structure to the specified coordinates.
 * 
 * This function takes an existing Text structure, copies it, and then
 * updates the position coordinates to the provided x and y values.
 * The modified Text structure is returned.
 * 
 * @param cText The original Text structure to be repositioned.
 * @param x The new x-coordinate for the Text.
 * @param y The new y-coordinate for the Text.
 * 
 * @return A Text structure with updated position coordinates.
 */
Text rePosText(Text cText, int x, int y)
{
    Text text = cText;
    text.pnt.x = x;
    text.pnt.y = y;
    return text;
}

/**
 * @brief Initializes a Rect structure with the specified parameters.
 * 
 * This function creates a Rect structure and initializes its attributes,
 * including the position, size, color, and returns the initialized structure.
 * The resulting Rect structure can be used for rendering rectangles.
 * 
 * @param x The x-coordinate of the rectangle.
 * @param y The y-coordinate of the rectangle.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @param color The color of the rectangle.
 * 
 * @return A Rect structure initialized with the specified parameters.
 */
Rect initRect(int x, int y, int w, int h, Color color)
{
    Rect rect;
    rect.pnt.x = x;
    rect.pnt.y = y;
    rect.size.w = w;
    rect.size.h = h;
    rect.color = color;
    return rect;
}

/**
 * @brief Repositions a Rect structure to the specified coordinates and updates its size.
 * 
 * This function takes an existing Rect structure, copies it, and then
 * updates the position coordinates, width, and height to the provided values.
 * The modified Rect structure is returned.
 * 
 * @param cRect The original Rect structure to be repositioned and resized.
 * @param x The new x-coordinate for the Rect.
 * @param y The new y-coordinate for the Rect.
 * @param w The new width for the Rect.
 * @param h The new height for the Rect.
 * 
 * @return A Rect structure with updated position coordinates and size.
 */
Rect rePosRect(Rect cRect, int x, int y, int w, int h)
{
    Rect rect = cRect;
    rect.pnt.x = x;
    rect.pnt.y = y;
    rect.size.w = w;
    rect.size.h = h;
    return rect;
}

/**
 * @brief Creates a Button structure with the specified parameters.
 * 
 * This function initializes a Button structure with the provided position,
 * size, string, colors, font, and callback function for the button click.
 * The resulting Button structure can be used for interactive buttons.
 * 
 * @param x The x-coordinate of the button.
 * @param y The y-coordinate of the button.
 * @param w The width of the button.
 * @param h The height of the button.
 * @param str The string to be displayed on the button.
 * @param color The color of the button.
 * @param textColor The color of the button text.
 * @param fnt The font for the button text.
 * @param click The callback function to be executed on button click.
 * 
 * @return A Button structure initialized with the specified parameters.
 */
Button createButton(int x, int y, int w, int h, char* str, Color color, Color textColor, Font fnt, void* click)
{
    Button button;
    button.pnt.x = x;
    button.pnt.y = y;
    button.size.w = w;
    button.size.h = h;
    button.color = color;
    button.textColor = textColor;
    button.func = click;
    button.font = fnt;

    // button text
    button.str = malloc(strlen(str) + 1);
    strcpy(button.str, str);
    button.length = strlen(str) + 1;

    return button;
}

/**
 * @brief Repositions a Button structure to the specified coordinates and updates its size.
 * 
 * This function takes an existing Button structure, copies it, and then
 * updates the position coordinates, width, and height to the provided values.
 * The modified Button structure is returned.
 * 
 * @param cButton The original Button structure to be repositioned and resized.
 * @param x The new x-coordinate for the Button.
 * @param y The new y-coordinate for the Button.
 * @param w The new width for the Button.
 * @param h The new height for the Button.
 * 
 * @return A Button structure with updated position coordinates and size.
 */
Button rePosButton(Button cButton, int x, int y, int w, int h)
{
    Button button = cButton;
    button.pnt.x = x;
    button.pnt.y = y;
    button.size.w = w;
    button.size.h = h;
    return button;
}

/**
 * @brief Creates a MessageBoxRend structure with the specified parameters.
 * 
 * This function initializes a MessageBoxRend structure with the provided
 * position, size, title, text, colors, font, and callback function for the button click.
 * The resulting MessageBoxRend structure can be used for displaying message boxes.
 * 
 * @param x The x-coordinate of the message box.
 * @param y The y-coordinate of the message box.
 * @param w The width of the message box.
 * @param h The height of the message box.
 * @param title The title text for the message box.
 * @param text The main text content of the message box.
 * @param color The color of the message box content.
 * @param textColor The color of the message box text.
 * @param fnt The font for the message box text.
 * @param click The callback function to be executed on button click.
 * 
 * @return A MessageBoxRend structure initialized with the specified parameters.
 */
MessageBoxRend createMessageBox(int x, int y, int w, int h, char* title, char* text, Color color, Color textColor, Font fnt, void* click)
{
    MessageBoxRend msgBox;
    msgBox.show = true;
    msgBox.background = initRect(x, y, w, h, (Color){255, 127, 127, 255});
    msgBox.foreground = initRect(x + 10, y + 10, w - 20, h - 20, color);
    msgBox.titleTextBackground = initRect(x, y, w, 40, (Color){96, 96, 96, 255});
    msgBox.title = initText(title, x + 20, y + 10, strlen(title) + 1, textColor, fnt);
    msgBox.text = initText(text, x + 20, y + 40, strlen(text) + 1, textColor, fnt);
    msgBox.xButton = createButton(x + w - 30, y + 10, 20, 20, "X", (Color){255, 64, 64, 255}, (Color){255, 255, 255, 255}, fnt, click);
    return msgBox;
}

/**
 * @brief Repositions a MessageBoxRend structure to the specified coordinates and updates its size.
 * 
 * This function takes an existing MessageBoxRend structure, copies it, and then
 * updates the position coordinates, width, and height to the provided values.
 * The modified MessageBoxRend structure is returned.
 * 
 * @param cmb The original MessageBoxRend structure to be repositioned and resized.
 * @param x The new x-coordinate for the MessageBoxRend.
 * @param y The new y-coordinate for the MessageBoxRend.
 * @param w The new width for the MessageBoxRend.
 * @param h The new height for the MessageBoxRend.
 * 
 * @return A MessageBoxRend structure with updated position coordinates and size.
 */
MessageBoxRend rePosMessageBox(MessageBoxRend cmb, int x, int y, int w, int h)
{
    MessageBoxRend msgBox = cmb;
    msgBox.show = true;
    msgBox.background = rePosRect(msgBox.background, x, y, w, h);
    msgBox.foreground = rePosRect(msgBox.foreground, x + 10, y + 10, w - 20, h - 20);
    msgBox.titleTextBackground = rePosRect(msgBox.titleTextBackground, x, y, w, 40);
    msgBox.title = rePosText(msgBox.title, x + 20, y + 10);
    msgBox.text = rePosText(msgBox.text, x + 20, y + 40);
    msgBox.xButton = rePosButton(msgBox.xButton, x + w - 30, y + 10, 20, 20);
    return msgBox;
}

/**
 * @brief Initializes a Texture structure from an image file.
 * 
 * This function creates a Texture structure and loads an image file
 * to associate with the texture. It also sets the texture's position,
 * size, rotation, and rotation around center flag.
 * 
 * @param win The window in which the texture will be rendered.
 * @param file The path to the image file.
 * @param x The x-coordinate of the texture.
 * @param y The y-coordinate of the texture.
 * @param w The width of the texture.
 * @param h The height of the texture.
 * @param rotation The rotation angle of the texture.
 * @param rotateACent If true, the texture rotates around its center; otherwise, it rotates around its top-left corner.
 * 
 * @return A Texture structure initialized with the specified parameters.
 */
Texture initTexture(Window win, char* file, int x, int y, int w, int h, float rotation, bool rotateACent)
{
    Texture tex;
    Color color = (Color){0xff,0xff,0xff,0xff};
    tex.rotation = rotation;
    tex.rotateAroundCenter = rotateACent;
    tex.color = color;

    FILE* f = fopen(file, "rb");
    if(!f)
    {
        char* errorText = malloc(100);
        memset(errorText, 0, 100);
        sprintf(errorText, "initTexture(): cannot open file '%s'.", file);
        problem(win, errorText, true);
    }
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* data = allocateMemory(size);
    fread(data, size, 1, f);
    fclose(f);
    
    #ifdef CC_BUILD_WIN
    // png/jpg/bmp data to SDL_Surface
    SDL_Surface* surface = IMG_Load_RW(SDL_RWFromMem(data, w * h * 4), 1);

    // set position
    tex.position.x = x;
    tex.position.y = y;

    // SDL_Surface to SDL_Texture
    tex.texture = SDL_CreateTextureFromSurface(win.renderer, surface);

    // free surface
    SDL_FreeSurface(surface);
    #endif

    // Wii rendering
    #ifdef CC_BUILD_GCWII
    tex.position.x = x;
    tex.position.y = y;
    tex.texture = GRRLIB_LoadTexture(data);
    #endif

    tex.size.w = w;
    tex.size.h = h;

    freeMemory(data);
    return tex;
}

/**
 * @brief Creates a Texture structure from image data.
 * 
 * This function creates a Texture structure and loads an image from the provided data.
 * It sets the texture's position, size, rotation, and rotation around center flag.
 * 
 * @param win The window in which the texture will be rendered.
 * @param data The image data.
 * @param size The size of the image data.
 * @param x The x-coordinate of the texture.
 * @param y The y-coordinate of the texture.
 * @param w The width of the texture.
 * @param h The height of the texture.
 * @param rotation The rotation angle of the texture.
 * @param rotateACent If true, the texture rotates around its center; otherwise, it rotates around its top-left corner.
 * 
 * @return A Texture structure initialized with the specified parameters.
 */
Texture loadTexture(Window win, unsigned char* data, int size, int x, int y, int w, int h, float rotation, bool rotateACent)
{
    Texture tex;
    Color color = (Color){0xff,0xff,0xff,0xff};
    tex.rotation = rotation;
    tex.rotateAroundCenter = rotateACent;
    tex.color = color;

    #ifdef CC_BUILD_WIN
    // png/jpg/bmp data to SDL_Surface
    SDL_Surface* surface = IMG_Load_RW(SDL_RWFromMem(data, w * h * 4), 1);

    // set position
    tex.position.x = x;
    tex.position.y = y;

    // SDL_Surface to SDL_Texture
    tex.texture = SDL_CreateTextureFromSurface(win.renderer, surface);

    // free surface
    SDL_FreeSurface(surface);
    #endif

    // Wii rendering
    #ifdef CC_BUILD_GCWII
    tex.position.x = x;
    tex.position.y = y;
    tex.texture = GRRLIB_LoadTexture(data);
    #endif

    tex.size.w = w;
    tex.size.h = h;

    return tex;
}

/**
 * @brief Converts a Rect structure to an Object structure.
 * 
 * This function creates an Object structure and initializes its position
 * and size based on the provided Rect structure.
 * 
 * @param rect The Rect structure to be converted to an Object.
 * 
 * @return An Object structure initialized with the Rect's position and size.
 */
Object* rectToObj(Rect rect)
{
    Object* obj = malloc(sizeof(Object));
    obj->pnt = rect.pnt;
    obj->size = rect.size;

    return obj;
}

/**
 * @brief Converts a Color structure to a 32-bit unsigned integer.
 * 
 * This function takes a Color structure and converts it to a 32-bit
 * unsigned integer, where each color component corresponds to a byte.
 * 
 * @param color The Color structure to be converted.
 * 
 * @return A 32-bit unsigned integer representing the Color.
 */
u32 ColorToU32(Color color)
{
    int r,g,b,a;
    r = color.r;
    g = color.g;
    b = color.b;
    a = color.a;
    return (r<<24) | (g<<16) | (b<<8) | (a&0xff);
}

/**
 * @brief Renders a texture onto a window.
 * 
 * This function renders a texture onto the specified window. The rendering
 * process varies between Windows and Wii platforms, supporting rotation.
 * 
 * @param tex The Texture structure to be rendered.
 * @param win The Window structure onto which the texture will be rendered.
 * 
 * @return void
 */
void renderTexture(Texture tex, Window win) {
    // Windows rendering
    #ifdef CC_BUILD_WIN
    // Create a destination rectangle for rendering
    SDL_Rect destRect;
    destRect.x = tex.position.x;
    destRect.y = tex.position.y;
    destRect.w = tex.size.w;
    destRect.h = tex.size.h;

    // Create a rotation point (optional)
    SDL_Point rotationPoint;
    if (tex.rotateAroundCenter) {
        rotationPoint.x = tex.size.w / 2;
        rotationPoint.y = tex.size.h / 2;
    } else {
        rotationPoint.x = tex.position.x;
        rotationPoint.y = tex.position.y;
    }

    // Render the texture with rotation
    SDL_SetTextureColorMod(tex.texture, tex.color.r, tex.color.g, tex.color.b); // Set the color modulation (tint) for the texture
    SDL_SetTextureAlphaMod(tex.texture, tex.color.a); // Set the alpha modulation for the texture

    SDL_RenderCopyEx(win.renderer, tex.texture, NULL, &destRect, tex.rotation, &rotationPoint, SDL_FLIP_NONE);

    // Reset the color and alpha modulation to default values
    SDL_SetTextureColorMod(tex.texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(tex.texture, 255);
    #endif

    // Wii rendering
    #ifdef CC_BUILD_GCWII
    // Calculate rotation in radians
    float rotationRadians = tex.rotation * M_PI / 180.0f;

    // Determine the rotation center based on the provided rotation point
    int rotationCenterX = tex.rotateAroundCenter ? tex.size.w / 2 : 0;
    int rotationCenterY = tex.rotateAroundCenter ? tex.size.h / 2 : 0;

    // Calculate the new position based on rotation around rotation center
    float cosTheta = cosf(rotationRadians);
    float sinTheta = sinf(rotationRadians);

    int drawX = rotationCenterX - (rotationCenterX * cosTheta - rotationCenterY * sinTheta);
    int drawY = rotationCenterY - (rotationCenterX * sinTheta + rotationCenterY * cosTheta);

    // Render the texture with manual rotation using GRRLIB_DrawImg
    GRRLIB_DrawImg(tex.position.x + drawX, tex.position.y + drawY, tex.texture, tex.rotation, 1, 1, ColorToU32(tex.color));
    #endif
}

/**
 * @brief Renders text onto a window.
 * 
 * This function renders text onto the specified window. The rendering
 * process varies between Windows and Wii platforms, supporting TrueType fonts.
 * 
 * @param window The Window structure onto which the text will be rendered.
 * @param text The Text structure containing the text and its attributes.
 * 
 * @return void
 */
void renderText(Window window, Text text)
{
    #ifdef CC_BUILD_WIN
        SDL_Surface* surface = TTF_RenderText_Solid(text.font.font, text.str, convertColor(text.color));
        SDL_Texture* texture = SDL_CreateTextureFromSurface(window.renderer, surface);
        SDL_Rect rect = {text.pnt.x, text.pnt.y, surface->w, surface->h};
        SDL_RenderCopy(window.renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    #endif
    // we must convert the color to a grrlib color
    #ifdef CC_BUILD_GCWII
        GRRLIB_PrintfTTF(text.pnt.x, text.pnt.y, text.font.font, text.str, text.font.size, ColorToU32(text.color));
    #endif
}

/**
 * @brief Renders a rectangle onto a window.
 * 
 * This function renders a rectangle onto the specified window. The rendering
 * process varies between Windows and Wii platforms, supporting color and size.
 * 
 * @param window The Window structure onto which the rectangle will be rendered.
 * @param rect The Rect structure defining the position, size, and color of the rectangle.
 * 
 * @return void
 */
void renderRect(Window window, Rect rect)
{
    #ifdef CC_BUILD_WIN
        SDL_SetRenderDrawColor(window.renderer, rect.color.r, rect.color.g, rect.color.b, rect.color.a);
        SDL_Rect sdlRect = {rect.pnt.x, rect.pnt.y, rect.size.w, rect.size.h};
        SDL_RenderFillRect(window.renderer, &sdlRect);
        //GRRLIB_Rectangle(rect.pnt.x, rect.pnt.y, rect.size.w, rect.size.h, rect.color, 1);
    #endif
    #ifdef CC_BUILD_GCWII
        GRRLIB_Rectangle(rect.pnt.x, rect.pnt.y, rect.size.w, rect.size.h, convertColor(rect.color), 1);
    #endif
}

/**
 * @brief Renders a button onto a window.
 * 
 * This function renders a button, represented as a rectangle with text, onto the
 * specified window. The rendering process varies between Windows and Wii platforms.
 * 
 * @param window The Window structure onto which the button will be rendered.
 * @param button The Button structure defining the button's attributes.
 * 
 * @return void
 */
void renderButton(Window window, Button button)
{
    renderRect(window, initRect(button.pnt.x, button.pnt.y, button.size.w, button.size.h, button.color));
    int textLengthInPixels;
    int textHeightInPixels;
    #ifdef CC_BUILD_WIN
        TTF_SizeText(button.font.font, button.str, &textLengthInPixels, &textHeightInPixels);
    #endif
    #ifdef CC_BUILD_GCWII
        textLengthInPixels = GRRLIB_WidthTTF(button.font.font, button.str, button.font.size);
        textHeightInPixels = button.font.size;
    #endif
    renderText(window, initText(button.str, button.pnt.x + button.size.w / 2 - textLengthInPixels / 2, button.pnt.y + button.size.h / 2 - textHeightInPixels/2, button.length, button.textColor, button.font));
}

/**
 * @brief Gets the width and height of the rendered text in pixels.
 * 
 * This function calculates and returns the width and height of the rendered
 * text in pixels using the provided Font and Text structures.
 * 
 * @param font The Font structure defining the font attributes.
 * @param text The Text structure containing the text and its attributes.
 * 
 * @return A Size structure containing the width and height of the rendered text.
 */
Size getTextWidthTTF(Font font, Text text)
{
    Size ret;
    int textLengthInPixels;
    int textHeightInPixels;
    #ifdef CC_BUILD_WIN
        TTF_SizeText(font.font, text.str, &textLengthInPixels, &textHeightInPixels);
    #endif
    #ifdef CC_BUILD_GCWII
        textLengthInPixels = GRRLIB_WidthTTF(font.font, text.str, font.size);
        textHeightInPixels = font.size;
    #endif
    ret.w = textLengthInPixels;
    ret.h = textHeightInPixels;
    return ret;
}

/**
 * @brief Renders a message box on a window.
 * 
 * This function renders a message box on the specified window, including
 * background, foreground, title, text, and a close button. It also handles
 * the click event on the close button.
 * 
 * @param window The Window structure onto which the message box will be rendered.
 * @param msgBox The MessageBoxRend structure defining the message box's attributes.
 * @param mouseDown Indicates whether the mouse button is down.
 * @param mouseX The x-coordinate of the mouse cursor.
 * @param mouseY The y-coordinate of the mouse cursor.
 * 
 * @return void
 */
void renderMBox(Window window, MessageBoxRend msgBox, bool mouseDown, int mouseX, int mouseY)
{
    if(!msgBox.show)
        return;
    else {
        renderRect(window, msgBox.background);
        renderRect(window, msgBox.foreground);
        renderRect(window, msgBox.titleTextBackground);
        renderText(window, msgBox.title);
        renderText(window, msgBox.text);
        renderButton(window, msgBox.xButton);
        checkButtonClicked(msgBox.xButton, mouseDown, mouseX, mouseY, 0, NULL);
    }
}

/**
 * @brief Performs simple collision detection between two objects.
 * 
 * This function checks whether two objects are colliding and sets the xHit
 * and yHit variables accordingly.
 * 
 * @param sprite1 The first Object.
 * @param sprite2 The second Object.
 * @param xHit A pointer to the x-axis collision indicator.
 * @param yHit A pointer to the y-axis collision indicator.
 * 
 * @return void
 */
void simpleCollisionDetector(Object* sprite1, Object* sprite2, int *xHit, int *yHit)
{
    if (sprite1->pnt.x < sprite2->pnt.x + sprite2->size.w &&
        sprite1->pnt.x + sprite1->size.w > sprite2->pnt.x &&
        sprite1->pnt.y < sprite2->pnt.y + sprite2->size.h &&
        sprite1->pnt.y + sprite1->size.h > sprite2->pnt.y)
    {
        *xHit = sprite1->pnt.x < sprite2->pnt.x + sprite2->size.w &&
                sprite1->pnt.x + sprite1->size.w > sprite2->pnt.x;
        *yHit = sprite1->pnt.y < sprite2->pnt.y + sprite2->size.h &&
                sprite1->pnt.y + sprite1->size.h > sprite2->pnt.y;
    }
}

// the above is sort of deprecated, but it's still used in some places so I'm keeping it for now
// the below is the new way of doing things (where more then just a sprite can collide, not just sprites)
// so instead of passing in two sprites, you pass in two objects, and the function will check if they are colliding
// and if they are, it will set the xHit and yHit variables to true. If they are not colliding, it will set them to false.
/**
 * @brief Performs collision detection between two objects.
 * 
 * This function checks whether two objects are colliding and sets the xHit
 * and yHit variables accordingly.
 * 
 * @param obj1 The first Object.
 * @param obj2 The second Object.
 * @param xHit A pointer to the x-axis collision indicator.
 * @param yHit A pointer to the y-axis collision indicator.
 * 
 * @return void
 */
void collisionDetector(Object* obj1, Object* obj2, int *xHit, int *yHit)
{
    if (obj1->pnt.x < obj2->pnt.x + obj2->size.w &&
        obj1->pnt.x + obj1->size.w > obj2->pnt.x &&
        obj1->pnt.y < obj2->pnt.y + obj2->size.h &&
        obj1->pnt.y + obj1->size.h > obj2->pnt.y)
    {
        *xHit = obj1->pnt.x < obj2->pnt.x + obj2->size.w &&
                obj1->pnt.x + obj1->size.w > obj2->pnt.x;
        *yHit = obj1->pnt.y < obj2->pnt.y + obj2->size.h &&
                obj1->pnt.y + obj1->size.h > obj2->pnt.y;
    }
}

// the above but a bool instead of two ints (returns if its colliding or not)
/**
 * @brief Performs collision detection between two objects and returns a boolean result.
 * 
 * This function checks whether two objects are colliding and returns a boolean result.
 * 
 * @param obj1 The first Object.
 * @param obj2 The second Object.
 * 
 * @return true if the objects are colliding, false otherwise.
 */
bool collisionDetectorBool(Object* obj1, Object* obj2)
{
    if (obj1->pnt.x < obj2->pnt.x + obj2->size.w &&
        obj1->pnt.x + obj1->size.w > obj2->pnt.x &&
        obj1->pnt.y < obj2->pnt.y + obj2->size.h &&
        obj1->pnt.y + obj1->size.h > obj2->pnt.y)
    {
        return true;
    }
    return false;
}

/**
 * @brief Checks if a button is clicked and executes its associated function.
 * 
 * This function checks if the specified button is clicked based on the mouse coordinates
 * and mouse button state. If the button is clicked, it executes the associated function.
 * 
 * @param button The Button structure defining the button's attributes.
 * @param mouseDown Indicates whether the mouse button is down.
 * @param mouse_x The x-coordinate of the mouse cursor.
 * @param mouse_y The y-coordinate of the mouse cursor.
 * @param argc The number of command line arguments.
 * @param argv The array of command line arguments.
 * 
 * @return void
 */
void checkButtonClicked(Button button, bool mouseDown, int mouse_x, int mouse_y, int argc, char** argv)
{
    if (mouse_x > button.pnt.x && mouse_x < button.pnt.x + button.size.w &&
        mouse_y > button.pnt.y && mouse_y < button.pnt.y + button.size.h && mouseDown)
    {
        button.func(argc, argv);
    }
}

float angle2 = 0.0f;

/**
 * @brief Clears the rendering screen.
 * 
 * This function clears the rendering screen on the specified window.
 * 
 * @param window The Window structure representing the rendering window.
 * 
 * @return void
 */
void clearScreen(Window window)
{
    #ifdef CC_BUILD_WIN
    SDL_RenderClear(window.renderer);
    #endif
    #ifdef CC_BUILD_GCWII
    GRRLIB_FillScreen(0x00000000);
    #endif
}

/**
 * @brief Fills the rendering screen with a specified color.
 * 
 * This function fills the rendering screen on the specified window
 * with the specified color.
 * 
 * @param win The Window structure representing the rendering window.
 * @param color The Color structure defining the fill color.
 * 
 * @return void
 */
void fillScreen(Window win, Color color)
{
    #ifdef CC_BUILD_WIN
        SDL_SetRenderDrawColor(win.renderer, color.r, color.g, color.b, color.a);
        SDL_Rect fillRect = {0, 0, win.width, win.height};
        SDL_RenderFillRect(win.renderer, &fillRect);
    #endif
    #ifdef CC_BUILD_GCWII
        GRRLIB_FillScreen(ColorToU32(color));
    #endif
}

/**
 * @brief Sets the rendering color on the window.
 * 
 * This function sets the rendering color on the specified window.
 * 
 * @param window The Window structure representing the rendering window.
 * @param color The Color structure defining the rendering color.
 * 
 * @return void
 */
void setRenderColor(Window window, Color color)
{
    #ifdef CC_BUILD_WIN
    SDL_SetRenderDrawColor(window.renderer, color.r, color.g, color.b, color.a);
    #endif
    //GRRLIB_SetColor(convertColor(color));
    #ifdef CC_BUILD_GCWII
    GRRLIB_SetBackgroundColour(color.r, color.g, color.b, color.a);
    #endif
}

/**
 * @brief Updates the rendering window.
 * 
 * This function updates the rendering window by presenting the renderer's
 * content, clearing the renderer, and updating the window surface.
 * 
 * @param win The Window structure representing the rendering window.
 * 
 * @return void
 */
void updateWindow(Window win) {
    // update window
    #ifdef CC_BUILD_WIN
    SDL_UpdateWindowSurface(win.window);
    SDL_RenderPresent(win.renderer);
    SDL_RenderClear(win.renderer);
    #endif

    #ifdef CC_BUILD_GCWII
    GRRLIB_Render();
    //VIDEO_WaitVSync();
    #endif
}

// 3d

// heres our crap to render a 3d object. (yes rendering in 3d is so much more complicated then 2d that were making the renderer before the object)
/**
 * @brief Renders a 3D mesh on the window.
 * 
 * This function renders a 3D mesh on the specified window using either
 * GRRLIB for the Nintendo Wii or OpenGL for other platforms.
 * 
 * @param win The Window structure representing the rendering window.
 * @param mesh The Mesh structure defining the 3D mesh and its attributes.
 * 
 * @return void
 */
void renderMesh(Window win, Mesh mesh)
{
    #ifdef CC_BUILD_GCWII
    // GRRLIB rendering code
    // Loop through the vertices and draw them
    if(mesh.textured == true)
    {
        // set the texture to the mesh
        GRRLIB_SetTexture(mesh.texture, 0);
    }

    // set the object's position, rotation, and scale with grrlib's functions
    GRRLIB_ObjectView(mesh.position.x, mesh.position.y, mesh.position.z, mesh.rotation.x, mesh.rotation.y, mesh.rotation.z, mesh.scale.x, mesh.scale.y, mesh.scale.z);
    GX_Begin(GX_QUADS, GX_VTXFMT0, mesh.vertexCount); // start drawing quads (4 sided shapes)

    for(int i = 0; i < mesh.vertexCount; i++)
    {
        GX_Position3f32(mesh.vertex[i].vertex[0], mesh.vertex[i].vertex[1], mesh.vertex[i].vertex[2]); // set the position of the vertex
        if(mesh.textured == true)
        {
            GX_Color1u32(0xFFFFFFFF); // set the color of the vertex
            GX_TexCoord2f32(mesh.vertex[i].texturePos.x, mesh.vertex[i].texturePos.y); // set the texture coordinates of the vertex
        }
        else
        {
            GX_Color1u32(mesh.vertex[i].color); // set the color of the vertex
        }
    }
    GX_End();
    #else
    // OpenGL rendering code
    // Loop through the vertices and draw them
    if (mesh.textured == true) {
        glBindTexture(GL_TEXTURE_2D, mesh.texture);
    }

    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glScalef(0.1f, 0.1f, 0.1f);
    glTranslatef(mesh.position.x, mesh.position.y, mesh.position.z);
    glRotatef(mesh.rotation.x, 1.0f, 0.0f, 0.0f);
    glRotatef(mesh.rotation.y, 0.0f, 1.0f, 0.0f);
    glRotatef(mesh.rotation.z, 0.0f, 0.0f, 1.0f);
    gluPerspective(45.0f, (GLfloat)win.width / (GLfloat)win.height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    //glScalef(mesh.scale.x, mesh.scale.y, mesh.scale.z);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_QUADS);
    for (int i = 0; i < mesh.vertexCount; i++) {
        glVertex3f(mesh.vertex[i].vertex[0], mesh.vertex[i].vertex[1], mesh.vertex[i].vertex[2]);
        
        if (mesh.textured == true) {
            glTexCoord2f(mesh.vertex[i].texturePos.x, mesh.vertex[i].texturePos.y);
        }
        
        //glColor3ub((mesh.vertex[i].color >> 24) & 0xFF, (mesh.vertex[i].color >> 16) & 0xFF, (mesh.vertex[i].color >> 8) & 0xFF);
        glColor3ub(rand()%0xff, rand()%0xff, rand()%0xff);
    }
    glEnd();

    glPopMatrix();

    if (mesh.textured == true) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    SDL_GL_SwapWindow(win.window);
    #endif
}

/**
 * @brief Renders a 3D model on the window.
 * 
 * This function renders a 3D model on the specified window by iterating through
 * its meshes and rendering each one using the renderMesh function.
 * 
 * @param win The Window structure representing the rendering window.
 * @param model The Model structure defining the 3D model and its meshes.
 * 
 * @return void
 */
void renderModel(Window win, Model model)
{
    // Loop through the meshes and draw them
    for(int i = 0; i < model.meshCount; i++)
    {
        renderMesh(win, model.mesh[i]);
    }
}

// save the model to a file function
/**
 * @brief Saves a 3D model to a binary file.
 * 
 * This function saves a 3D model to a binary file, including mesh count,
 * mesh details, and optionally textures.
 * 
 * @param model The Model structure representing the 3D model to be saved.
 * @param file The name of the binary file to save the model to.
 * 
 * @return void
 */
void saveModel(Model model, char* file)
{
    FILE* fp = fopen(file, "wb");
    // write the model to the file in binary
    fwrite(&model.meshCount, sizeof(int), 1, fp);
    for(int i = 0; i < model.meshCount; i++)
    {
        fwrite(&model.mesh[i].vertexCount, sizeof(int), 1, fp);
        fwrite(&model.mesh[i].textured, sizeof(bool), 1, fp);
        if(model.mesh[i].textured)
        {
            // save the texture size
            //fwrite(&model.mesh[i].textureSize, sizeof(int), 1, fp);
            // write the size of the temporary texture
            // load a temporary texture from a file
            FILE* texture = fopen(model.mesh[i].textureFile, "rb");
            fseek(texture, 0, SEEK_END);
            int textureSize = ftell(texture);
            fseek(texture, 0, SEEK_SET);
            char* textureData = malloc(textureSize);
            fread(textureData, sizeof(u8), textureSize, texture);
            fclose(texture);
            // save the texture
            fwrite(&textureSize, sizeof(int), 1, fp);
            fwrite(textureData, textureSize, 1, fp);
        }
        for(int j = 0; j < model.mesh[i].vertexCount; j++)
        {
            fwrite(&model.mesh[i].vertex[j].vertex, sizeof(f32), 3, fp);
            fwrite(&model.mesh[i].vertex[j].color, sizeof(u32), 1, fp);
            fwrite(&model.mesh[i].vertex[j].texturePos, sizeof(Vector2), 1, fp);
        }
        fwrite(&model.mesh[i].position, sizeof(Vector3), 1, fp);
        fwrite(&model.mesh[i].rotation, sizeof(Vector3), 1, fp);
        fwrite(&model.mesh[i].scale, sizeof(Vector3), 1, fp);
    }
    fclose(fp);
}


// load the model from a file function
/**
 * @brief Loads a 3D model from a binary file.
 * 
 * This function loads a 3D model from a binary file, including mesh count,
 * mesh details, and optionally textures.
 * 
 * @param file The name of the binary file to load the model from.
 * 
 * @return The loaded Model structure.
 */
Model loadModel(char* file)
{
    FILE* fp = fopen(file, "rb");
    if(!fp)
    {
        char* errorText = malloc(100);
        memset(errorText, 0, 100);
        sprintf(errorText, "initTexture(): cannot open file '%s'.", file);
        problem(win, errorText, true);
    }
    Model model;
    // read the model from the file in binary
    fread(&model.meshCount, sizeof(int), 1, fp);
    model.mesh = malloc(sizeof(Mesh)-sizeof(model.mesh->texture) * model.meshCount);
    for(int i = 0; i < model.meshCount; i++)
    {
        fread(&model.mesh[i].vertexCount, sizeof(int), 1, fp);
        fread(&model.mesh[i].textured, sizeof(bool), 1, fp);
        if(model.mesh[i].textured)
        {
            // get the texture size
            fread(&model.mesh[i].textureSize, sizeof(int), 1, fp);
            char *texture = malloc(model.mesh[i].textureSize);
            fread(texture, model.mesh[i].textureSize, 1, fp);
            //model.mesh[i].texture = GRRLIB_LoadTexture(texture);
            //model.mesh[i].texture = GRRLIB_LoadTextureFromSd(getFile("girl.png"));
            free(texture);
        }
        model.mesh[i].vertex = malloc(sizeof(Vertex) * model.mesh[i].vertexCount);
        for(int j = 0; j < model.mesh[i].vertexCount; j++)
        {
            fread(&model.mesh[i].vertex[j].vertex, sizeof(f32), 3, fp);
            fread(&model.mesh[i].vertex[j].color, sizeof(u32), 1, fp);
            fread(&model.mesh[i].vertex[j].texturePos, sizeof(Vector2), 1, fp);
        }
        fread(&model.mesh[i].position, sizeof(Vector3), 1, fp);
        fread(&model.mesh[i].rotation, sizeof(Vector3), 1, fp);
        fread(&model.mesh[i].scale, sizeof(Vector3), 1, fp);
    }
    fclose(fp);
    return model;
}