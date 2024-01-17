#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
//#include <endian.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include <MPR/SAM/reciter.h>
#include <MPR/SAM/sam.h>
#include <MPR/SAM/debug.h>
#include <MPR/SAM/libsam.h>

#include <MPR/sound.h>

int debug = 0;

int MPR_Sam(int argc, char **argv)
{
    int i;
    int phonetic = 0;

    //char input[256];

    //for(i=0; i<256; i++) input[i] = 0;

    char input[256] = "Test, One, Two, Three.\0";

    for(i=0; input[i] != 0; i++)
        input[i] = toupper((int)input[i]);

    if (debug)
    {
        if (phonetic) printf("phonetic input: %s\n", input);
        else printf("text input: %s\n", input);
    }

    if (!phonetic)
    {
        strncat(input, "[", 256);
        if (!TextToPhonemes((unsigned char *)input)) printf("error: !TextToPhonemes((unsigned char *)input))\n");
        if (debug)
            printf("phonetic input: %s\n", input);
    } else strncat(input, "\x9b", 256);


    SetInput(input);
    if (!SAMMain())
    {
        return 1;
    }

    playBuffer(GetBuffer(), GetBufferLength()/50, 22050/2, 1);

    return 0;
}
