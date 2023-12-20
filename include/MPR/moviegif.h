/**
 * @brief A simple gif player with audio support written for MPRenderer.
 * @author Dakota Thorpe.
 * @file moviegif.h
*/
#ifndef MOVIEGIF_H
#define MOVIEGIF_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include <MPR/gifdec.h>
#include <MPR/memory.h>
#include <MPR/sound.h>
#include <MPR/core.h>

/**
 * @brief The audio types.
*/
enum {
    MOGI_AUDIO_OGG,
    MOGI_AUDIO_PCM
};

/**
 * @brief Our MovieGif header.
*/
typedef struct
{
    /* Magic and Version */
    char magic[4];
    int version;
    
    /* Gif Details. */
    uint16_t gif_width;
    uint16_t gif_height;
    uint8_t gif_framerate;
    uint32_t gif_frames;
    uint64_t gif_dataSize;

    /* Audio Details */
    uint8_t audio_type;
    uint16_t audio_bitrate;
    uint8_t audio_channels;
    uint64_t audio_dataSize;
} moviegif_header_t;

/**
 * @brief Our MovieGif type.
*/
typedef struct {
    moviegif_header_t meta; // Metadata.
    gd_GIF* gifdata;
    uint8_t* audiodata;
} MovieGif_t;

void MovieGif_CreateMovieFromFiles(char* gifFile, char* audioFile, char* outputFile, uint8_t audiotype, uint16_t audio_bitrate, uint8_t audio_channels);
MovieGif_t LoadMovieGif(char* filename);
void MovieGif_TestPlayback(Window win, MovieGif_t movie);

#endif