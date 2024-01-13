#include <cs50.h>
#include <stdio.h>

int getCollatzStepsCount(int n, int stepsCount) {
    if(n == 1)
        return stepsCount;

    // if is even
    if(n % 2 == 0)
        return getCollatzStepsCount(n/2, stepsCount + 1);

    // if is odd
    return getCollatzStepsCount((3 * n) + 1, stepsCount + 1);
}

int main(void) {
    // get start number from user
    int n;
    do {
        n = get_int("Enter a number: ");
    } while(n < 1);

    printf("It takes %i step(s) to get to 1\n", getCollatzStepsCount(n, 0));
}
