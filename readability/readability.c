#include <cs50.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>

/*
Consider letters to be uppercase
or lowercase alphabetical character,
not punctuation, digits, or other symbols.

Assumes text contains atleast one character
*/
int countLetters(string text)
{
    int count = 0;
    int i = 0;
    do
    {
        if (isalpha(text[i]))
            count++;
    }
    while (text[++i] != '\0');

    return count;
}

/*
A word is a sequence of characters separated by a space
e.g. a hyphenated word like "sister-in-law" should be
considered one word, not three

Assumes that a sentence:
    - will contain at least one word;
    - will not start or end with a space; and
    - will not have multiple spaces in a row.
*/
int countWords(string text)
{
    // assume atleast one word
    // NOTE: we do this as a space signifies
    //       the existence of the next word,
    //       not the current word, so we need to
    //       account for the first word which
    //       doesnt have a preceeding space
    int count = 1;
    int i = 0;
    do
    {
        if (text[i] == ' ')
            count++;
    }
    while (text[++i] != '\0');

    return count;
}

bool isSentenceEndChar(char character)
{
    return character == '.' || character == '!' || character == '?';
}

/*
A sentence is any sequence of characters
that ends with a '.' or a '!' or a '?'

In practice, sentence boundary detection
needs to be a little more intelligent
to handle edge cases but here its acceptable.
*/
int countSentences(string text)
{
    int count = 0;
    int i = 0;
    do
    {
        if (isSentenceEndChar(text[i]))
            count++;
    }
    while (text[++i] != '\0');

    return count;
}

/*
Calculates the Coleman-Liau index (ie the reading grade)
using the formula "index = 0.0588 * L - 0.296 * S - 15.8"
Where:
    - L = the average number of letters per 100 words
    - S = the average number of sentences per 100 words
*/
int calculateReadingGrade(float letterCount, float wordCount, float sentenceCount)
{
    float L = 100 * (letterCount / wordCount);
    float S = 100 * (sentenceCount / wordCount);
    float grade = (0.0588 * L) - (0.296 * S) - 15.8;
    return round(grade); // need to round instead of truncating
}

/*
Prints the grade as "Grade [grade]"
where [grade] is rounded to the nearest integer.

If the resulting index number is 16 or higher
then "Grade 16+" should be printed.

If the index number is less than 1 then
"Before Grade 1" should be printed.
*/
void printReadingGrade(int grade)
{
    if (grade >= 16)
        printf("Grade 16+\n");

    else if (grade < 1)
        printf("Before Grade 1\n");

    else
        printf("Grade %i\n", grade);
}

int main(void)
{
    const string TEXT = get_string("Text: ");
    const int LETTER_COUNT = countLetters(TEXT);
    const int WORD_COUNT = countWords(TEXT);
    const int SENTENCE_COUNT = countSentences(TEXT);
    const int GRADE = calculateReadingGrade(LETTER_COUNT, WORD_COUNT, SENTENCE_COUNT);
    printReadingGrade(GRADE);
}
