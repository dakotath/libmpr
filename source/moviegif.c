/**
 * @file moviegif.c
 * @details A simple gif player with audio support written for MPRenderer.
 * @author Dakota Thorpe.
 * @date 2023-12-19
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#ifdef CC_BUILD_WIN
#include <windows.h>
#define gettime() GetTickCount()
#endif

#include <MPR/moviegif.h>
#include <MPR/gifdec.h>
#include <MPR/memory.h>
#include <MPR/sound.h>
#include <MPR/core.h>
#include <MPR/input.h>

char MOVIEGIF_MAGIC[4] = {'M','O','G','I'}; // Our Magic Identifier.
int MOVIEGIF_VERSION = 1; // Movie gif version number.

/**
 * @brief Creates a MovieGif file from a GIF and audio file.
 * 
 * This function creates a MovieGif file from a GIF file and an audio file, storing the result in an output file.
 * 
 * @param gifFile Path to the input GIF file.
 * @param audioFile Path to the input audio file.
 * @param outputFile Path to the output MovieGif file.
 * @param audiotype Type of the audio.
 * @param audio_bitrate Bitrate of the audio.
 * @param audio_channels Number of audio channels.
 * 
 * @return void
 */
void MovieGif_CreateMovieFromFiles(char* gifFile, char* audioFile, char* outputFile, uint8_t audiotype, uint16_t audio_bitrate, uint8_t audio_channels)
{
    // Create the data parts.
    MovieGif_t movieGif;
    moviegif_header_t movieHeader;

    // Set magic and version.
    movieHeader.version = MOVIEGIF_VERSION;
    for(int i=0; i<4; i++)
    {
        movieHeader.magic[i] = MOVIEGIF_MAGIC[i];
    }

    // Load in our files.
    FILE* GifInput;
    FILE* AudioInput;
    GifInput = fopen(gifFile, "rb"); // Open Gif.
    AudioInput = fopen(audioFile, "rb"); // Open Audio.

    // Lets get the meta of our gif.
    movieGif.gifdata = gd_open_gif(gifFile);
    movieHeader.gif_width = movieGif.gifdata->width;
    movieHeader.gif_height = movieGif.gifdata->height;

    // Set audio.
    movieHeader.audio_type = audiotype;
    movieHeader.audio_bitrate = audio_bitrate;
    movieHeader.audio_channels = audio_channels;

    // Set size of gif data.
    uint64_t gifdata_size;
    fseek(GifInput, 0, SEEK_END);
    gifdata_size = ftell(GifInput);
    fseek(GifInput, 0, SEEK_SET);
    movieHeader.gif_dataSize = gifdata_size;

    // Get audio size.
    uint64_t audiodata_size;
    fseek(AudioInput, 0, SEEK_END);
    audiodata_size = ftell(AudioInput);
    fseek(AudioInput, 0, SEEK_SET);
    movieHeader.audio_dataSize = audiodata_size;

    // Save the MovieGif file.
    FILE* SaveFile = fopen(outputFile, "wb");

    // Write meta.
    fwrite(&movieHeader, sizeof(moviegif_header_t), 1, SaveFile); // Save our header.

    // Write the gif and audio data.
    uint8_t* gifData = (uint8_t*)allocateMemory(gifdata_size); // Allocate memory for gif.
    uint8_t* audioData = (uint8_t*)allocateMemory(audiodata_size); // Allocate memory for audio.
    
    fread(gifData, gifdata_size, 1, GifInput); // Read gif data into the gif data buffer.
    fread(audioData, audiodata_size, 1, AudioInput); // Read audio data into the audio data buffer.

    fwrite(gifData, gifdata_size, 1, SaveFile); // Write gif to SaveFile.
    fwrite(audioData, audiodata_size, 1, SaveFile); // Write audio to SaveFile.

    // Clean up and close.
    freeMemory(gifData);
    freeMemory(audioData);

    fclose(GifInput);
    fclose(AudioInput);
    fclose(SaveFile);
    gd_close_gif(movieGif.gifdata);

    return;
}

/**
 * @brief Loads a MovieGif from a file.
 * 
 * This function loads a MovieGif from a file and returns the loaded data.
 * 
 * @param filename Path to the MovieGif file.
 * 
 * @return MovieGif_t struct containing the loaded data.
 */
MovieGif_t LoadMovieGif(char* filename)
{
    FILE* inputFile = fopen(filename, "rb");

    // Read into a meta.
    moviegif_header_t metadata;
    fread(&metadata, sizeof(moviegif_header_t), 1, inputFile);

    // Load in the raw data's.
    uint8_t* gifData = (uint8_t*)allocateMemory(metadata.gif_dataSize); // Allocate memory for gif.
    uint8_t* audioData = (uint8_t*)allocateMemory(metadata.audio_dataSize); // Allocate memory for audio.
    fread(gifData, metadata.gif_dataSize, 1, inputFile);
    fread(audioData, metadata.audio_dataSize, 1, inputFile);
    
    // Save the gif to sd.
    FILE* tmpGif = fopen("tmpGif.gif", "wb");
    fwrite(gifData, metadata.gif_dataSize, 1, tmpGif);
    fclose(tmpGif);

    // Load in the gif.
    MovieGif_t retData;
    retData.meta = metadata;
    retData.gifdata = gd_open_gif("tmpGif.gif");
    retData.audiodata = audioData;

    freeMemory(gifData);
    fclose(inputFile);

    return retData;
}

/**
 * @brief Converts ticks to milliseconds.
 * 
 * This function converts ticks to milliseconds.
 * 
 * @param ticks Ticks to be converted.
 * 
 * @return Converted milliseconds.
 */
u32 ticks_to_millisecs(uint64_t ticks) {
    return ticks / 1000ULL;
}

/**
 * @brief Renders a frame from a GIF onto a window.
 * 
 * This function renders a frame from a GIF onto a window.
 * 
 * @param win Window to render onto.
 * @param tex Texture to render onto.
 * @param gif GIF data.
 * @param scale_factor Scaling factor for the rendered frame.
 * 
 * @return void
 */
void renderFrame(Window win, void* tex, gd_GIF *gif, int scale_factor) {
    uint8_t *color, *frame;
    int i, j;

    frame = malloc(gif->width * gif->height * 3);
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        return;
    }

    int ret = gd_get_frame(gif);
    if (ret == -1) {
        free(frame);
        return;
    }

    gd_render_frame(gif, frame);
    color = frame;

    Rect rectAngle;
    Vector2 pos;
    Size size;
    Color pixcolor;

    for (i = 0; i < gif->height; i++) {
        for (j = 0; j < gif->width; j++) {
            if (!gd_is_bgcolor(gif, color)) {
                pos.x = j * scale_factor;
                pos.y = i * scale_factor;

                size.w = scale_factor;
                size.h = scale_factor;

                pixcolor.r = color[0];
                pixcolor.g = color[1];
                pixcolor.b = color[2];
                pixcolor.a = 0xff;

                rectAngle.pnt = pos;
                rectAngle.size = size;
                rectAngle.color = pixcolor;
                
                renderRect(win, rectAngle);
            }
            color += 3;
        }
    }

    free(frame);
}

/**
 * @brief Tests the playback of a MovieGif.
 * 
 * This function tests the playback of a MovieGif on a given window.
 * 
 * @param win Window to render onto.
 * @param movie MovieGif data.
 * 
 * @return void
 */
void MovieGif_TestPlayback(Window win, MovieGif_t movie)
{
    int ret, paused, quit;
    uint32_t t0, t1, delay, delta;

    switch (movie.meta.audio_type)
    {
    case MOGI_AUDIO_OGG:
        /* code */
        playOgg(movie.audiodata, movie.meta.audio_dataSize, 0, 1);
        break;
    
    default:
        break;
    }

    #ifdef CC_BUILD_GCWII
    GRRLIB_texImg *tex = GRRLIB_CreateEmptyTexture(movie.gifdata->width, movie.gifdata->height);
    #elif defined(CC_BUILD_WIN)
    SDL_Texture *tex = SDL_CreateTexture(win.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, movie.gifdata->width, movie.gifdata->height);
    #else
    void *tex;
    #endif

    paused = 0;
    quit = 0;

    int scaleFact = 4;
    while (!checkKey(KEY_MENU)) {
        t0 = ticks_to_millisecs(gettime());

        fillScreen(win, (Color){0xa8,0xa8,0xa8,0xff});
        renderFrame(win, tex, movie.gifdata, scaleFact);
        updateWindow(win);

        t1 = ticks_to_millisecs(gettime());
        delta = t1 - t0;
        delay = movie.gifdata->gce.delay * 10;
        delay = delay > delta ? delay - delta : 1;

        wait(delay);  // Convert delay to microseconds

        ret = gd_get_frame(movie.gifdata);
        if (ret == 0) {
            gd_rewind(movie.gifdata);
        }
    }
}