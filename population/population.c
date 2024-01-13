#include <cs50.h>
#include <stdio.h>

// NOTE: output made to fit expected check50 format, but more descriptive messages included in comments
int main(void)
{
    // Prompt for start size
    int currentPopulation;
    do
    {
        currentPopulation = get_int("How many llamas are there initially? ");
        /*
        if (currentPopulation < 9)
        {
            printf("\tWarning: Initial population must be atleast 9\n");
        }
        */
    }
    while (currentPopulation < 9);

    // Prompt for end size
    int targetPopulation;
    do
    {
        targetPopulation = get_int("What is the target population? ");
        /*
        if (targetPopulation < currentPopulation)
        {
            printf("\tWarning: Target population must be greater than or equal to the initial population (%i)\n",
        currentPopulation);
        }
        */
    }
    while (targetPopulation < currentPopulation);

    // Calculate number of years until we reach threshold
    int years = 0;
    while (currentPopulation < targetPopulation)
    {
        years++;

        // NOTE: integer division will trim any fractional values from the result, ie we only count "full" llamas
        // NOTE: cant simplify this to just add 1/12th (ie 1/3 born - 1/4 pass away = 4/12 - 3/12 = 1/12 new llamas overall)
        //       as this doesnt consider the truncating effect for each operation
        currentPopulation += (currentPopulation / 3) - (currentPopulation / 4);
        // printf("Population after %i year(s) is %i\n", years, currentPopulation);
    }

    // Print number of years
    // printf("Target population of %i will be reached in %i year(s)\n", targetPopulation, years);
    printf("Years: %i\n", years);
}
