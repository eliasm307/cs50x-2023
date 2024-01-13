import cs50


# Determines the type of card based on the digit count and prefix number criterias for each card
def get_card_type(cardNumberAsString):
    digitsCount = len(cardNumberAsString)
    firstTwoDigits = int(cardNumberAsString[:2])

    if digitsCount == 15 and firstTwoDigits in [34, 37]:
        return "AMEX"

    if digitsCount == 16 and firstTwoDigits in range(51, 56):
        return "MASTERCARD"

    if digitsCount in [13, 16] and cardNumberAsString[0] == "4":
        return "VISA"

    return "INVALID"


# Returns the maximum power of 10 that fits into the given number,
# e.g. "300" would return "2" as "10^2" ie "100" is the max that can fit
def sum_digits(number):
    sum = 0
    for c in str(number):
        sum += int(c)

    return sum


# Applies Luhn’s Algorithm to determine whether the given card number is valid
def checksum_is_valid(cardNumberAsString):
    checkSum = 0
    length = len(cardNumberAsString)

    for n in range(1, length + 1):
        # number is in big endian, ie the most signifcant digit first,
        # so we need to iterate through the numbers backwards
        # in order to know what is actually an even/odd placed digit in the number
        # relative to the start (ie where the least significant digit is)
        digit = int(cardNumberAsString[length - n])
        if n % 2 == 0:
            checkSum += sum_digits(digit * 2)
        else:
            checkSum += sum_digits(digit)

    return checkSum % 10 == 0


#####################################
# MAIN SCRIPT


cardNumberAsString = str(cs50.get_int("Number: "))

cardType = get_card_type(cardNumberAsString)

if cardType != "INVALID" and not checksum_is_valid(cardNumberAsString):
    cardType = "INVALID"

print(cardType)
