#include <cs50.h>
#include <stdio.h>

void printCharNTimes(string value, int repeatCount)
{
    for (int i = 0; i < repeatCount; i++)
    {
        printf("%s", value);
    }
}

int main(void)
{
    // Prompt user for required height
    int height;
    do
    {
        height = get_int("How high do you want the pyramid (from 1 to 8)? ");
    }
    while (height < 1 || height > 8);

    // define variables for readability
    const int PYRAMID_COLUMNS_COUNT = height;
    const int GAP_COLUMNS_COUNT = 2;
    const int OVERALL_COLUMNS_COUNT = (height * 2) + GAP_COLUMNS_COUNT;

    // print pyramids line by line
    for (int lineIndex = 0; lineIndex < height; lineIndex++)
    {
        const int BLOCK_COUNT_AT_CURRENT_LINE = lineIndex + 1;

        // print gap before the first left pyramid block
        printCharNTimes(" ", PYRAMID_COLUMNS_COUNT - BLOCK_COUNT_AT_CURRENT_LINE);

        // print left pyramid blocks
        printCharNTimes("#", BLOCK_COUNT_AT_CURRENT_LINE);

        // print gap between pyramids
        printCharNTimes(" ", GAP_COLUMNS_COUNT);

        // print right pyramid blocks
        printCharNTimes("#", BLOCK_COUNT_AT_CURRENT_LINE);

        printf("\n");
    }
}
