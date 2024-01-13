#include <cs50.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Points assigned to each letter of the alphabet
int POINTS[] = {1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10};

int compute_score(string word);

int main(void)
{
    // Get input words from both players
    string word1 = get_string("Player 1: ");
    string word2 = get_string("Player 2: ");

    // Score both words
    int score1 = compute_score(word1);
    int score2 = compute_score(word2);

    // check for a tie
    if (score1 == score2)
    {
        printf("Tie!\n");
        return 0;
    }

    // determine which player won
    int winningPlayer = 1;
    if (score2 > score1)
    {
        winningPlayer = 2;
    }
    printf("Player %i wins!\n", winningPlayer);
}

// Computes and returns score for string
int compute_score(string word)
{
    int score = 0;
    int length = strlen(word);
    for (int i = 0; i < length; i++)
    {
        // get and normalise characters to lower case
        char character = tolower(word[i]);

        // only score alphabet characters
        if (character >= 'a' && character <= 'z')
        {
            // add the character score via the character index, ie where 'a' is 0
            score += POINTS[character - 'a'];
        }
    }
    return score;
}
