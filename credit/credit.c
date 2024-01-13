#include <cs50.h>
#include <stdio.h>
#include <string.h>

// NOTE: assumes power is minimum 0
long tenToThePower(int power)
{
    long outputValue = 1;
    for (int i = 0; i < power; i++)
    {
        outputValue *= 10;
    }
    return outputValue;
}

// Sums the digits of the given number, e.g. "12" would return "3"
int sumDigits(int number)
{
    int sumOfDigits = 0;
    long currentValue = number;
    for (int power = (number / 10); power >= 0; power--)
    {
        const long TEN_TO_THE_CURRENT_POWER = tenToThePower(power);
        const int DIGIT_AT_CURRENT_POWER = currentValue / TEN_TO_THE_CURRENT_POWER;
        sumOfDigits += DIGIT_AT_CURRENT_POWER;
        currentValue -= DIGIT_AT_CURRENT_POWER * TEN_TO_THE_CURRENT_POWER;
    }
    return sumOfDigits;
}

// Returns the maximum power of 10 that fits into the given number,
// e.g. "300" would return "2" as "10^2" ie "100" is the max that can fit
int getMaxPowerOfTen(long number)
{
    int powerOfTen = 0;
    long powerOfTenValue = 1;
    while (powerOfTenValue < number)
    {
        powerOfTen++;
        powerOfTenValue *= 10;
    }

    // NOTE: the power would be the one that produces a value over the input,
    //       ie the value is the power just below that
    return powerOfTen - 1;
}

// Determines the type of card based on the digit count and prefix number criterias for each card
string getCardType(long cardNumber)
{
    int maxPowerOfTen = getMaxPowerOfTen(cardNumber);
    long maxPowerOfTenValue = tenToThePower(maxPowerOfTen);
    int firstDigit = cardNumber / maxPowerOfTenValue;
    int secondDigit = 10 * (cardNumber - (firstDigit * maxPowerOfTenValue)) / maxPowerOfTenValue;
    int firstTwoDigits = (firstDigit * 10) + secondDigit;
    const int DIGITS_COUNT = maxPowerOfTen + 1;
    if (DIGITS_COUNT == 15 && (firstTwoDigits == 34 || firstTwoDigits == 37))
        return "AMEX";

    if (DIGITS_COUNT == 16 && (firstTwoDigits >= 51 && firstTwoDigits <= 55))
        return "MASTERCARD";

    if ((DIGITS_COUNT == 13 || DIGITS_COUNT == 16) && firstDigit == 4)
        return "VISA";

    return "INVALID";
}

// applies Luhn’s Algorithm to determine whether the given card number is valid
bool checkSumIsValid(long cardNumber)
{
    int checkSum = 0;
    long currentValue = cardNumber;
    for (int power = getMaxPowerOfTen(cardNumber); power >= 0; power--)
    {
        const long TEN_TO_THE_CURRENT_POWER = tenToThePower(power);
        const int DIGIT_AT_CURRENT_POWER = currentValue / TEN_TO_THE_CURRENT_POWER;
        if (power % 2 == 0)
            checkSum += sumDigits(DIGIT_AT_CURRENT_POWER);
        else
            checkSum += sumDigits(DIGIT_AT_CURRENT_POWER * 2);

        currentValue -= DIGIT_AT_CURRENT_POWER * TEN_TO_THE_CURRENT_POWER;
    }

    // if it is a multiple of 10 without remainders then it ends with a 0
    return checkSum % 10 == 0;
}

int main(void)
{
    // Prompt user for input (assumes card numbers do not start with leading zeros)
    const long INPUT = get_long("Please enter a credit card number: ");

    // determine the card type
    string cardType = getCardType(INPUT);

    // if card type is valid but checksum is not valid, then result is not valid
    if (strcmp(cardType, "INVALID") != 0 && !checkSumIsValid(INPUT))
    {
        cardType = "INVALID";
    }

    printf("%s\n", cardType);
}
