#include "helpers.h"
#include <math.h>

// type to store sum of multiple signed values that may be larger than 255 e.g. sum before averaging
typedef struct
{
    int rgbtBlue;
    int rgbtGreen;
    int rgbtRed;
} __attribute__((__packed__)) RGBTRIPLE_INT;

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    for (int rowIndex = 0; rowIndex < height; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < width; columnIndex++)
        {
            // calculate the average from the RGB values, and assign this to the red colour
            image[rowIndex][columnIndex].rgbtRed =
                roundf((image[rowIndex][columnIndex].rgbtBlue + image[rowIndex][columnIndex].rgbtGreen +
                        image[rowIndex][columnIndex].rgbtRed) /
                       3.0);

            // assign average colour to blue and green also (not doing this in the same line as average calculation for readability)
            image[rowIndex][columnIndex].rgbtBlue = image[rowIndex][columnIndex].rgbtGreen = image[rowIndex][columnIndex].rgbtRed;
        }
    }
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    // NOTE: int division rounds down the fractional half of odd numbers,
    //       ie we ignore the center pixel which doesnt actually get reflected
    const int HALF_WIDTH = width / 2;
    RGBTRIPLE tempPixel;
    for (int rowIndex = 0; rowIndex < height; rowIndex++)
    {
        // for each row, mirror/swap the pixels in the left half with the pixels in the right half
        for (int columnIndex = 0; columnIndex < HALF_WIDTH; columnIndex++)
        {
            tempPixel = image[rowIndex][columnIndex];
            image[rowIndex][columnIndex] = image[rowIndex][width - 1 - columnIndex];
            image[rowIndex][width - 1 - columnIndex] = tempPixel;
        }
    }
}

void addPixelColoursToTargetWithWeight(RGBTRIPLE *source, RGBTRIPLE_INT *target, int weight)
{
    (*target).rgbtRed += (*source).rgbtRed * weight;
    (*target).rgbtGreen += (*source).rgbtGreen * weight;
    (*target).rgbtBlue += (*source).rgbtBlue * weight;
}

BYTE capColourValue(int val)
{
    if (val > 255)
        return 255; // cap max to max byte value

    if (val < 0)
        return 0; // cap min to 0

    return val;
}

BYTE getAverageColour(int total, int count)
{
    return capColourValue(roundf((float) total / count));
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE tempImage[height][width];
    // sum of colours from neighbouring pixels and the current pixel
    RGBTRIPLE_INT total;
    // count of the neighbouring pixels and the current pixel
    int pixelsCount;
    for (int rowIndex = 0; rowIndex < height; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < width; columnIndex++)
        {
            // set current pixel colours as initial values for total
            total.rgbtRed = image[rowIndex][columnIndex].rgbtRed;
            total.rgbtGreen = image[rowIndex][columnIndex].rgbtGreen;
            total.rgbtBlue = image[rowIndex][columnIndex].rgbtBlue;
            pixelsCount = 1;

            // if has row above
            if ((rowIndex - 1) >= 0)
            {
                // add top center pixel colours
                addPixelColoursToTargetWithWeight(&image[rowIndex - 1][columnIndex], &total, 1);
                pixelsCount++;

                // add top left pixel colours, if it has column to the left
                if ((columnIndex - 1) >= 0)
                {
                    addPixelColoursToTargetWithWeight(&image[rowIndex - 1][columnIndex - 1], &total, 1);
                    pixelsCount++;
                }

                // add top right pixel colours, if it has column to the right
                if ((columnIndex + 1) < width)
                {
                    addPixelColoursToTargetWithWeight(&image[rowIndex - 1][columnIndex + 1], &total, 1);
                    pixelsCount++;
                }
            }

            // if has row below
            if ((rowIndex + 1) < height)
            {
                // add bottom center pixel colours
                addPixelColoursToTargetWithWeight(&image[rowIndex + 1][columnIndex], &total, 1);
                pixelsCount++;

                // add bottom left pixel colours, if it has column to the left
                if ((columnIndex - 1) >= 0)
                {
                    addPixelColoursToTargetWithWeight(&image[rowIndex + 1][columnIndex - 1], &total, 1);
                    pixelsCount++;
                }

                // add bottom right pixel colours, if it has column to the right
                if ((columnIndex + 1) < width)
                {
                    addPixelColoursToTargetWithWeight(&image[rowIndex + 1][columnIndex + 1], &total, 1);
                    pixelsCount++;
                }
            }

            // add center left pixel colours, if it has column to the left
            if ((columnIndex - 1) >= 0)
            {
                addPixelColoursToTargetWithWeight(&image[rowIndex][columnIndex - 1], &total, 1);
                pixelsCount++;
            }

            // add center right pixel colours, if it has column to the right
            if ((columnIndex + 1) < width)
            {
                addPixelColoursToTargetWithWeight(&image[rowIndex][columnIndex + 1], &total, 1);
                pixelsCount++;
            }

            // average colours and apply them to pixel in temp image
            // need to do this on temp image so the full image can be processed with the original values
            tempImage[rowIndex][columnIndex].rgbtRed = getAverageColour(total.rgbtRed, pixelsCount);
            tempImage[rowIndex][columnIndex].rgbtGreen = getAverageColour(total.rgbtGreen, pixelsCount);
            tempImage[rowIndex][columnIndex].rgbtBlue = getAverageColour(total.rgbtBlue, pixelsCount);
        }
    }

    // overwrite image pixels with updated temp image pixels
    for (int rowIndex = 0; rowIndex < height; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < width; columnIndex++)
        {
            image[rowIndex][columnIndex] = tempImage[rowIndex][columnIndex];
        }
    }
}

// combines Gx and Gy into a final value by calculating the square root of Gx^2 + Gy^2
BYTE getSobelOperatorFinalColourValue(int gxValue, int gyValue)
{
    return capColourValue(roundf(sqrt(pow(gxValue, 2) + pow(gyValue, 2))));
}

// Overwrites the given pixel's colours with the calculated Sobel operator values
void applySobelOperatorToPixel(RGBTRIPLE *target, RGBTRIPLE_INT *gx, RGBTRIPLE_INT *gy)
{
    (*target).rgbtRed = getSobelOperatorFinalColourValue((*gx).rgbtRed, (*gy).rgbtRed);
    (*target).rgbtGreen = getSobelOperatorFinalColourValue((*gx).rgbtGreen, (*gy).rgbtGreen);
    (*target).rgbtBlue = getSobelOperatorFinalColourValue((*gx).rgbtBlue, (*gy).rgbtBlue);
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE_INT gx;
    RGBTRIPLE_INT gy;
    RGBTRIPLE tempImage[height][width];

    for (int rowIndex = 0; rowIndex < height; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < width; columnIndex++)
        {
            // reset values
            gx.rgbtRed = 0;
            gx.rgbtGreen = 0;
            gx.rgbtBlue = 0;

            gy.rgbtRed = 0;
            gy.rgbtGreen = 0;
            gy.rgbtBlue = 0;

            // if has row above
            if ((rowIndex - 1) >= 0)
            {
                // add top center pixel colours
                // (gx weight is 0 for this pixel)
                addPixelColoursToTargetWithWeight(&image[rowIndex - 1][columnIndex], &gy, -2);

                // add top left pixel colours, if it has column to the left
                if ((columnIndex - 1) >= 0)
                {
                    addPixelColoursToTargetWithWeight(&image[rowIndex - 1][columnIndex - 1], &gx, -1);
                    addPixelColoursToTargetWithWeight(&image[rowIndex - 1][columnIndex - 1], &gy, -1);
                }

                // add top right pixel colours, if it has column to the right
                if ((columnIndex + 1) < width)
                {
                    addPixelColoursToTargetWithWeight(&image[rowIndex - 1][columnIndex + 1], &gx, 1);
                    addPixelColoursToTargetWithWeight(&image[rowIndex - 1][columnIndex + 1], &gy, -1);
                }
            }
            // else assume pixels are black (ie 0 for all colours) so dont change gx or gy

            // if has row below
            if ((rowIndex + 1) < height)
            {
                // add bottom center pixel colours
                // (gx weight is 0 for this pixel)
                addPixelColoursToTargetWithWeight(&image[rowIndex + 1][columnIndex], &gy, 2);

                // add bottom left pixel colours, if it has column to the left
                if ((columnIndex - 1) >= 0)
                {
                    addPixelColoursToTargetWithWeight(&image[rowIndex + 1][columnIndex - 1], &gx, -1);
                    addPixelColoursToTargetWithWeight(&image[rowIndex + 1][columnIndex - 1], &gy, 1);
                }

                // add bottom right pixel colours, if it has column to the right
                if ((columnIndex + 1) < width)
                {
                    addPixelColoursToTargetWithWeight(&image[rowIndex + 1][columnIndex + 1], &gx, 1);
                    addPixelColoursToTargetWithWeight(&image[rowIndex + 1][columnIndex + 1], &gy, 1);
                }
            }

            // add center left pixel colours, if it has column to the left
            if ((columnIndex - 1) >= 0)
            {
                addPixelColoursToTargetWithWeight(&image[rowIndex][columnIndex - 1], &gx, -2);
                // (gy weight is 0 for this pixel)
            }

            // add center right pixel colours, if it has column to the right
            if ((columnIndex + 1) < width)
            {
                addPixelColoursToTargetWithWeight(&image[rowIndex][columnIndex + 1], &gx, 2);
                // (gy weight is 0 for this pixel)
            }

            // add output to temp image, need to do this on temp image so the full image can be processed with the original values
            applySobelOperatorToPixel(&tempImage[rowIndex][columnIndex], &gx, &gy);
        }
    }

    // overwrite image pixels with updated temp image pixels
    for (int rowIndex = 0; rowIndex < height; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < width; columnIndex++)
        {
            image[rowIndex][columnIndex] = tempImage[rowIndex][columnIndex];
        }
    }
}
