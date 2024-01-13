// Implements a dictionary's functionality

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "dictionary.h"

// Represents a node in a hash table
typedef struct node
{
    char word[LENGTH + 1];
    struct node *next;
} node;

const unsigned int SUM_OF_POSITION_NUMBERS = (LENGTH * (LENGTH + 1)) / 2;

// Choose number of buckets in hash table
// (ie the max hash value, ie for a max length string of all 'z' characters)
// This is based on the hash algorithm where each char value (ie 'a' is 1)
// is multiplied by its position (ie first position is 1)
// and the overall length of the word is added.
// This sacrifices memory efficiency to improve lookup speed
const unsigned int N = (26 * SUM_OF_POSITION_NUMBERS) + LENGTH;

// Hash table
node *table[N];

// Returns true if word is in dictionary, else false
bool check(const char *word)
{
    // get head node in related hash bucket
    node *nodePtr = table[hash(word)];

    // check if words in hash bucket node list match target word
    while (nodePtr != NULL)
    {
        if (strcasecmp(word, nodePtr->word) == 0)
            return true; // word match found

        // check the next node
        nodePtr = nodePtr->next;
    }

    // word not found in bucket
    return false;
}

// Hashes word to a number
unsigned int hash(const char *word)
{
    unsigned int i = 0;
    unsigned int hashValue = 0;
    while (word[i] != '\0')
    {
        if (word[i] == '\'')
            // NOTE: considering apostrophe as a negative shift
            //       assumes this never causes the overall hash to be negative
            hashValue += -1;

        else
            // NOTE: considers characters (case insensitive) as numerical values, ie 'a' is 1,
            //       and also considers the ordering of the characters, ie first position i 1,
            //       ie multiplies the character value by its position
            hashValue += (tolower(word[i]) - 'a' + 1) * (i + 1);

        i++; // look at next char
    }

    // consider string length
    // NOTE: in this case i is the index of the '\0' char, ie the length of the string content
    hashValue += i;

    return hashValue;
}

// Loads dictionary into memory, returning true if successful, else false
// NOTE: dictionary argument is path to dictionary file
bool load(const char *dictionary)
{
    // open dictionary file
    FILE *file = fopen(dictionary, "r");
    if (file == NULL)
    {
        printf("Could not open dictionary: %s.\n", dictionary);
        return false;
    }

    node *newNodePtr = NULL;
    unsigned int hashValue;
    // char index in the current word
    int wordIndex = 0;
    // buffer which will be loaded with chars read from the dictionary
    char charBuffer;
    while (fread(&charBuffer, sizeof(char), 1, file) == 1)
    {
        // skip new lines
        if (charBuffer == '\n')
            continue;

        // allocate space for new word node
        newNodePtr = malloc(sizeof(node));

        // handle memory allocation issue
        if (newNodePtr == NULL)
        {
            unload(); // unload nodes loaded so far
            fclose(file);
            return false; // indicate load error
        }

        // read entire word (including apostrophes)
        wordIndex = 0;
        while (isalpha(charBuffer) || (charBuffer == '\''))
        {
            newNodePtr->word[wordIndex++] = charBuffer;
            if (fread(&charBuffer, sizeof(char), 1, file) != 1)
                break; // likely end of file
        }

        // add string end marker
        newNodePtr->word[wordIndex] = '\0';

        // add word to table (ie at the beginning of linked list)
        hashValue = hash(&newNodePtr->word[0]);
        newNodePtr->next = table[hashValue]; // point new node next to current bucket head
        table[hashValue] = newNodePtr;       // move bucket head to new node
    }

    fclose(file);

    // loaded entire dictionary without issue
    return true;
}

// Returns number of words in dictionary if loaded, else 0 if not yet loaded
unsigned int size(void)
{
    unsigned int nodeCount = 0;
    node *currentNodePtr;
    // for each hash bucket
    for (int i = 0; i < N; i++)
    {
        // nodeCount nodes in linked list
        currentNodePtr = table[i];
        while (currentNodePtr != NULL)
        {
            nodeCount++;
            currentNodePtr = currentNodePtr->next;
        }
    }

    return nodeCount;
}

void freeNodeList(node *n)
{
    if (n == NULL)
        return;

    // free the next node before the current node,
    // to not lose the reference to the next node
    freeNodeList(n->next);
    free(n);
}

// Unloads dictionary from memory, returning true if successful, else false
bool unload(void)
{
    // free all nodes in all hash buckets
    for (int i = 0; i < N; i++)
    {
        freeNodeList(table[i]);
    }

    // NOTE: assumes is always successful, as #free
    //       does not provide feedback of success or failure
    return true;
}
