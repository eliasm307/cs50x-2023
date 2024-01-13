#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
} pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
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
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Returns -1 if candidate with given name not found
int get_candidate_index(string name)
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(candidates[i], name) == 0)
            return i;
    }
    return -1;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    int candidateIndex = get_candidate_index(name);
    if (candidateIndex == -1)
        return false; // candidate with given name not found

    ranks[rank] = candidateIndex;
    return true;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    // for each rank from the first to second to last, as the last one will not have any preferences to add in this case
    for (int currentRank = 0; currentRank < candidate_count - 1; currentRank++)
    {
        int currentRankCandidateIndex = ranks[currentRank];

        // for each rank below/after the current rank
        for (int lowerRank = currentRank + 1; lowerRank < candidate_count; lowerRank++)
        {
            int lowerRankCandidateIndex = ranks[lowerRank];

            // record current voter as preferring the candidate at the current rank over the one at the given lower rank
            preferences[currentRankCandidateIndex][lowerRankCandidateIndex]++;
        }
    }
}

// NOTE: does not add pair if it is a tie
// NOTE: Increases pair_count if a new pair is added
void determine_winner_and_add_pair(int candidateAIndex, int candidateBIndex)
{
    int votesForCandidateAOverCandidateB = preferences[candidateAIndex][candidateBIndex];
    int votesForCandidateBOverCandidateA = preferences[candidateBIndex][candidateAIndex];

    if (votesForCandidateAOverCandidateB == votesForCandidateBOverCandidateA)
        return; // prefences are equal, ie we dont add a pair or change the pair count

    // add a new pair, where pair_count represents the next available pair index
    // NOTE: it would be easier to calculate the victory strength here as an attribute of pair, but assuming assignment doesnt allow
    // modifying the struct
    if (votesForCandidateAOverCandidateB > votesForCandidateBOverCandidateA)
    {
        pairs[pair_count].winner = candidateAIndex;
        pairs[pair_count].loser = candidateBIndex;
    }
    else
    {
        pairs[pair_count].winner = candidateBIndex;
        pairs[pair_count].loser = candidateAIndex;
    }

    // we added a pair so pair count has increased
    pair_count++;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    // for each candidate except one, as the last one will have been compared to all other candidates
    for (int candidateAIndex = 0; candidateAIndex < candidate_count - 1; candidateAIndex++)
    {
        // for each candidate index after the current one, compare it to the current candidate and add winner/loser pair if
        // necessary
        for (int candidateBIndex = candidateAIndex + 1; candidateBIndex < candidate_count; candidateBIndex++)
        {
            determine_winner_and_add_pair(candidateAIndex, candidateBIndex);
        }
    }
}

int get_pair_victory_strength(pair p)
{
    return preferences[p.winner][p.loser];
}

// NOTE: indices are inclusive
// NOTE: results in pairs array being sorted
void merge_sort_pairs_between_indices(int startIndex, int endIndex)
{
    if (startIndex == endIndex)
        return; // base case, single element is sorted

    // NOTE: this is the last index of the left side
    // NOTE: this implicitly rounds down
    int middleIndex = (endIndex - startIndex) / 2;

    // sort the left side
    merge_sort_pairs_between_indices(startIndex, middleIndex);

    // copy left side
    // NOTE: +1 as middleIndex is included in left side
    const int leftSideLength = middleIndex - startIndex + 1;
    pair sortedLeftSidePairs[leftSideLength];
    for (int i = 0; i < leftSideLength; i++)
    {
        sortedLeftSidePairs[i] = pairs[i + startIndex];
    }

    // sort the right side
    merge_sort_pairs_between_indices(middleIndex + 1, endIndex);

    // copy right side
    // NOTE: no +1 as middleIndex is not included in right side
    const int rightSideLength = endIndex - middleIndex;
    pair sortedRightSidePairs[rightSideLength];
    for (int i = 0; i < rightSideLength; i++)
    {
        sortedRightSidePairs[i] = pairs[i + middleIndex + 1];
    }

    // merge left and right sides, overwriting the range in the pairs array
    int leftIndex = 0;
    int rightIndex = 0;
    const int allPairsCount = endIndex - startIndex + 1;
    for (int i = 0; i < allPairsCount; i++)
    {
        // get current left side pair victory strength
        int leftPairStrength = -1;
        if (leftIndex < leftSideLength)
            leftPairStrength = get_pair_victory_strength(sortedLeftSidePairs[leftIndex]);

        // get current right side pair victory strength
        int rightPairStrength = -1;
        if (rightIndex < rightSideLength)
            rightPairStrength = get_pair_victory_strength(sortedRightSidePairs[rightIndex]);

        // pick strongest pair
        // NOTE: if the pairs are equal strength then add the right pair
        if (leftPairStrength > rightPairStrength)
        {
            pairs[i + startIndex] = sortedLeftSidePairs[leftIndex];
            leftIndex++;
        }
        else
        {
            pairs[i + startIndex] = sortedRightSidePairs[rightIndex];
            rightIndex++;
        }
    }
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    merge_sort_pairs_between_indices(0, pair_count - 1);
}

// determines if the given source candidate directly
// or indirectly beats the given target candidate when looking at the locked in edges
bool candidate_is_overall_locked_in_to_beat(int sourceCandidateIndex, int targetCandidateIndex)
{
    // base case, source and destination have a directly locked in pair
    if (locked[sourceCandidateIndex][targetCandidateIndex])
        return true;

    // check for indirect lock in of source candidate beating target candidate
    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[sourceCandidateIndex][i] && candidate_is_overall_locked_in_to_beat(i, targetCandidateIndex))
            // if the source candidate is locked in to beat the active candidate (ie at the current index)
            // and if the active candidate is locked in to beat the target candidate
            // then the source candidate is indirectly locked in to beat the target candidate
            return true;
    }

    return false;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    for (int i = 0; i < pair_count; i++)
    {
        // only lock in pairs where the inverse has not already been locked in directly or indirectly
        if (!candidate_is_overall_locked_in_to_beat(pairs[i].loser, pairs[i].winner))
            locked[pairs[i].winner][pairs[i].loser] = true;
    }
}

// assume there will not be more than one source
int get_winner_candidate_index()
{
    // create array to track candidate losses
    int candidateLosses[candidate_count];
    for (int i = 0; i < candidate_count; i++)
    {
        candidateLosses[i] = 0;
    }

    // count locked in losses for each candidate
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            if (locked[i][j])
                // ie candidate at index j is locked in to lose, so count this loss
                candidateLosses[j]++;
        }
    }

    // find the candidate with no losses, ie the source with no nodes pointing to it
    for (int i = 0; i < candidate_count; i++)
    {
        if (candidateLosses[i] == 0)
            return i;
    }

    return -1; // Should not happen so we return a dummy index
}

// Print the winner of the election
void print_winner(void)
{
    int winnerIndex = get_winner_candidate_index();
    printf("%s\n", candidates[winnerIndex]);
}
