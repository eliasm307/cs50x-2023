// Modifies the volume of an audio file

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Number of bytes in .wav header
const int HEADER_SIZE = 44;

int main(int argc, char *argv[])
{
    // Check command-line arguments
    if (argc != 4)
    {
        printf("Usage: ./volume input.wav output.wav factor\n");
        return 1;
    }

    // Open files and determine scaling factor
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        printf("Could not open file.\n");
        return 1;
    }

    FILE *output = fopen(argv[2], "w");
    if (output == NULL)
    {
        printf("Could not open file.\n");
        fclose(input);
        return 1;
    }

    float factor = atof(argv[3]);

    // Copy header from input file
    uint8_t header[HEADER_SIZE];
    int actualSize = fread(header, sizeof(uint8_t), HEADER_SIZE, input);
    if (actualSize != HEADER_SIZE)
    {
        printf("Could not read input file header.\n");
        fclose(input);
        fclose(output);
        return 2;
    }

    // insert header into output file
    actualSize = fwrite(header, sizeof(uint8_t), HEADER_SIZE, output);
    if (actualSize != HEADER_SIZE)
    {
        printf("Could not write input file header to output file.\n");
        fclose(input);
        fclose(output);
        return 3;
    }

    // Read samples, one by one, from input file and write updated data to output file
    const int16_t MAX_16BIT_SIGNED_INT = 32767;
    const int16_t MIN_16BIT_SIGNED_INT = -32768;
    int16_t sampleBuffer16bit = 0;
    int32_t sampleBuffer32bit = 0; // NOTE: 32bit integer so we can limit this to 16 bit range after factor is applied
    // repeat while we can still read samples
    while (fread(&sampleBuffer16bit, sizeof(int16_t), 1, input) == 1)
    {
        // scale sample by input factor
        sampleBuffer32bit = sampleBuffer16bit * factor;

        // limit new sample to valid signed 16 bit value
        if (sampleBuffer32bit > MAX_16BIT_SIGNED_INT)
            sampleBuffer16bit = MAX_16BIT_SIGNED_INT;

        else if (sampleBuffer32bit < MIN_16BIT_SIGNED_INT)
            sampleBuffer16bit = MIN_16BIT_SIGNED_INT;

        else
            sampleBuffer16bit = sampleBuffer32bit;

        // write scaled sample to output file
        actualSize = fwrite(&sampleBuffer16bit, sizeof(int16_t), 1, output);
        if (actualSize != 1)
        {
            printf("Could not write sample to output file.\n");
            fclose(input);
            fclose(output);
            return 4;
        }
    }

    // Close files
    fclose(input);
    fclose(output);
}
