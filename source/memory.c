/**
 * @file memory.c
 * @details Memory functions.
 * @author Dakota Thorpe.
 * @date 2023-12-19
*/

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
#include <malloc.h>
// rendering for Windows is done with SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#endif

// custom stuff
#include <MPR/memory.h>

u64 availableMemory = TOTAL_AVAILABLE_MEMORY;
u64 usedMemory = 0;

/**
 * @brief Converts bytes to a human-readable string with appropriate units (B, KB, MB, GB, TB, PB).
 * 
 * This function converts a byte size to a human-readable string with appropriate units.
 * 
 * @param bytes Size in bytes.
 * 
 * @return Allocated string with the formatted size.
 */
char* bytes_to_KMGTP(u64 bytes) {
    char *output = malloc(32);
    if (bytes < 1024) {
        sprintf(output, "%llu B", bytes);
    } else if (bytes < 1024 * 1024) {
        sprintf(output, "%llu KB", bytes / 1024);
    } else if (bytes < 1024 * 1024 * 1024) {
        sprintf(output, "%llu MB", bytes / (1024 * 1024));
    } else if (bytes < 1024 * 1024 * 1024 * 1024) {
        sprintf(output, "%llu GB", bytes / (1024 * 1024 * 1024));
    } else if (bytes < 1024 * 1024 * 1024 * 1024 * 1024) {
        sprintf(output, "%llu TB", bytes / (1024 * 1024 * 1024 * 1024));
    } else {
        sprintf(output, "%llu PB", bytes / (1024 * 1024 * 1024 * 1024 * 1024));
    }

    return output;
}

/**
 * @brief Allocates memory and updates memory usage information.
 * 
 * This function allocates memory and updates memory usage information.
 * 
 * @param size Size of memory to allocate.
 * 
 * @return Pointer to the allocated memory.
 */
void* allocateMemory(u64 size) {
    if (size > availableMemory) {
        printf("Not enough memory available!\n");
        return NULL;
    }

    void* ptr = malloc(size);
    if (ptr == NULL) {
        printf("Failed to allocate memory!\n");
        return NULL;
    }

    availableMemory -= size;
    usedMemory += size;

    return ptr;
}

/**
 * @brief Allocates cleared memory and updates memory usage information.
 * 
 * This function allocates cleared memory (zero-initialized) and updates memory usage information.
 * 
 * @param arg1 First argument for size calculation.
 * @param arg2 Second argument for size calculation.
 * 
 * @return Pointer to the allocated cleared memory.
 */
void* allocateClearedMemory(u64 arg1, u64 arg2)
{
    u64 size = arg1*arg2;
    if (size > availableMemory) {
        printf("Not enough memory available!\n");
        return NULL;
    }

    void* ptr = malloc(size);
    if (ptr == NULL) {
        printf("Failed to allocate memory!\n");
        return NULL;
    }

    memset(ptr, 0, size);

    availableMemory -= size;
    usedMemory += size;

    return ptr;
}

/**
 * @brief Allocates aligned memory and updates memory usage information.
 * 
 * This function allocates aligned memory and updates memory usage information.
 * 
 * @param size Size of memory to allocate.
 * @param alignment Alignment of the memory.
 * 
 * @return Pointer to the allocated aligned memory.
 */
void* allocateMemoryAlligned(u64 size, u64 alignment) {
    if (size > availableMemory) {
        printf("Not enough memory available!\n");
        return NULL;
    }

    #ifdef CC_BUILD_GCWII
    void* ptr = memalign(alignment, size);
    #endif
    #ifdef CC_BUILD_WIN
    void* ptr = _aligned_malloc(size, alignment);
    #endif

    if (ptr == NULL) {
        printf("Failed to allocate memory!\n");
        return NULL;
    }

    availableMemory -= size;
    usedMemory += size;

    return ptr;
}

/**
 * @brief Frees memory and updates memory usage information.
 * 
 * This function frees memory and updates memory usage information.
 * 
 * @param ptr Pointer to the memory to free.
 * 
 * @return void
 */
void freeMemory(void* ptr) {
    if (ptr == NULL) {
        printf("Cannot free NULL pointer!\n");
        return;
    }

    // check the size of the allocated memory
    #ifdef CC_BUILD_GCWII
    u32 size = malloc_usable_size(ptr);
    #endif
    #ifdef CC_BUILD_WIN
    u32 size = _msize(ptr);
    #endif


    if(usedMemory - size < 0)
    {
        printf("Oh SHIT: UsedMemory-size = %s.\n", bytes_to_KMGTP(usedMemory - size));
        exit(-2);
    }

    free(ptr);

    availableMemory += size;
    usedMemory -= size;

    return;
}

/**
 * @brief Gets the available memory.
 * 
 * This function returns the available memory.
 * 
 * @return Available memory.
 */
u64 getAvailableMemory() {
    return availableMemory;
}

/**
 * @brief Gets the used memory.
 * 
 * This function returns the used memory.
 * 
 * @return Used memory.
 */
u64 getUsedMemory() {
    return usedMemory;
}

/**
 * @brief Prints memory usage information.
 * 
 * This function prints memory usage information to the console.
 * 
 * @return void
 */
void printUsage() {
    char* available = bytes_to_KMGTP(availableMemory);
    char* used = bytes_to_KMGTP(usedMemory);

    printf("usage: %s/%s\n", used, available);

    free(available);
    free(used);

    return;
}