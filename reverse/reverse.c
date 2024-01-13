#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "wav.h"

int check_format(WAVHEADER header);
int get_block_size(WAVHEADER header);

int main(int argc, char *argv[])
{
    // Ensure proper usage
    if (argc != 3)
    {
        printf("Usage: ./reverse input.wav output.wav\n");
        return 1;
    }

    // Open input file for reading
    FILE *inputFile = fopen(argv[1], "r");
    if (inputFile == NULL)
    {
        printf("Could not open input file.\n");
        return 2;
    }

    // Read header
    WAVHEADER header;
    const int HEADER_SIZE = sizeof(WAVHEADER);
    if (fread(&header, HEADER_SIZE, 1, inputFile) != 1)
    {
        printf("Could not read input file header.\n");
        fclose(inputFile);
        return 3;
    }

    // Use check_format to ensure WAV format
    if (!check_format(header))
    {
        printf("Unknown format.\n");
        return 4;
    }

    // Open output file for writing
    FILE *outputFile = fopen(argv[2], "w");
    if (outputFile == NULL)
    {
        printf("Could not open output file.\n");
        return 5;
    }

    // Write header to file
    if (fwrite(&header, HEADER_SIZE, 1, outputFile) != 1)
    {
        printf("Could not write input file header to output file.\n");
        fclose(inputFile);
        fclose(outputFile);
        return 6;
    }

    // Use get_block_size to calculate size of block
    const int BLOCK_SIZE = get_block_size(header);

    // Write reversed audio to file

    // go to the end of the input file
    if (fseek(inputFile, 0, SEEK_END) != 0)
    {
        printf("Could not seek input file end.\n");
        fclose(inputFile);
        fclose(outputFile);
        return 7;
    }

    uint8_t blockBuffer[BLOCK_SIZE];

    // while we are in the audio block after the header
    while (ftell(inputFile) > HEADER_SIZE)
    {
        // move before the current block (ie the block before the file cursor) so we can read it
        if (fseek(inputFile, -BLOCK_SIZE, SEEK_CUR) != 0)
        {
            printf("Could not seek backwards in input file.\n");
            fclose(inputFile);
            fclose(outputFile);
            return 8;
        }

        // read the current block (this will move the file cursor forward)
        if (fread(blockBuffer, sizeof(uint8_t), BLOCK_SIZE, inputFile) != BLOCK_SIZE)
        {
            printf("Could not read data from input.\n");
            fclose(inputFile);
            fclose(outputFile);
            return 9;
        }

        // move the file cursor back so we dont read the same block again on the next iteration
        if (fseek(inputFile, -BLOCK_SIZE, SEEK_CUR) != 0)
        {
            printf("Could not seek backwards in input file.\n");
            fclose(inputFile);
            fclose(outputFile);
            return 10;
        }

        // write the block to the new file in reverse order
        if (fwrite(blockBuffer, sizeof(uint8_t), BLOCK_SIZE, outputFile) != BLOCK_SIZE)
        {
            printf("Could not write data to output file.\n");
            fclose(inputFile);
            fclose(outputFile);
            return 11;
        }
    }

    fclose(inputFile);
    fclose(outputFile);
}

// checks file format block spells out ‘W’ ‘A’ ‘V’ ‘E’
int check_format(WAVHEADER header)
{
    return header.format[0] == 'W' && header.format[1] == 'A' && header.format[2] == 'V' && header.format[3] == 'E';
}

// block size in bytes
int get_block_size(WAVHEADER header)
{
    return header.numChannels * header.bitsPerSample / 8;
}
