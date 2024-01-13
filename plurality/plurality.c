#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// Candidates have name and vote count
typedef struct
{
    string name;
    int votes;
} candidate;

// Array of candidates
candidate candidates[MAX];

// Number of candidates
int candidate_count;

// Function prototypes
bool vote(string name);
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: plurality [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i].name = argv[i + 1];
        candidates[i].votes = 0;
    }

    int voter_count = get_int("Number of voters: ");

    // Loop over all voters
    for (int i = 0; i < voter_count; i++)
    {
        string name = get_string("Vote: ");

        // Check for invalid vote
        if (!vote(name))
        {
            printf("Invalid vote.\n");
        }
    }

    // Display winner of election
    print_winner();
}

// Update vote totals given a new vote
bool vote(string name)
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(candidates[i].name, name) == 0)
        {
            // increase candidate votes
            candidates[i].votes++;

            // exit early and signify the vote was successful
            return true;
        }
    }
    return false;
}

int get_winning_votes_count(void)
{
    int winning_votes_count = 0;
    for (int i = 0; i < candidate_count; i++)
    {
        // track what is the highest amount of votes received by any candidate
        if (candidates[i].votes > winning_votes_count)
            winning_votes_count = candidates[i].votes;
    }
    return winning_votes_count;
}

// Print the winner (or winners) of the election
void print_winner(void)
{
    int winning_votes_count = get_winning_votes_count();

    for (int i = 0; i < candidate_count; i++)
    {
        // print the winning candidates, ie with the leading vote counts
        if (candidates[i].votes == winning_votes_count)
            printf("%s\n", candidates[i].name);
    }
}
