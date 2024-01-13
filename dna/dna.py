import csv
import sys


def main():
    # Check for command-line usage
    if len(sys.argv) != 3:
        sys.exit("Usage: python dna.py DATABASE_FILENAME SEQUENCE_FILENAME")

    # Read database file into a variable
    # colum names: name,[Short Tandem Repeat (STR) sequences e.g. AGATC,AATG,TATC]
    str_counts = {}
    with open(sys.argv[1]) as file:
        for column_name in csv.DictReader(file).fieldnames:
            if column_name != "name":
                str_counts[column_name] = 0  # init dict with STR keys

    # Read DNA sequence file into a variable
    with open(sys.argv[2]) as file:
        sequence = file.read()

    # Find longest match of each STR in DNA sequence
    for str_sequence in str_counts:
        str_counts[str_sequence] = longest_match(sequence, str_sequence)

    # Check database for matching profiles
    with open(sys.argv[1]) as file:
        # check rows as we iterate them to (I assume) discard irrelevant rows or exit early if target is found
        # and to also avoid storing the whole database in memory
        for row in csv.DictReader(file):
            if database_row_is_match(row, str_counts) == True:
                print(row["name"])
                return  # match found, exit early

    print("No match")


def database_row_is_match(db_row, target_str_counts):
    for str_sequence in target_str_counts:
        if target_str_counts[str_sequence] != int(db_row[str_sequence]):
            return False

    return True  # all STR counts match


def longest_match(sequence, subsequence):
    """Returns length of longest run of subsequence in sequence."""

    # Initialize variables
    longest_run = 0
    subsequence_length = len(subsequence)
    sequence_length = len(sequence)

    # Check each character in sequence for most consecutive runs of subsequence
    for i in range(sequence_length):
        # Initialize count of consecutive runs
        count = 0

        # Check for a subsequence match in a "substring" (a subset of characters) within sequence
        # If a match, move substring to next potential match in sequence
        # Continue moving substring and checking for matches until out of consecutive matches
        while True:
            # Adjust substring start and end
            start = i + count * subsequence_length
            end = start + subsequence_length

            # If there is a match in the substring
            if sequence[start:end] == subsequence:
                count += 1

            # If there is no match in the substring
            else:
                break

        # Update most consecutive matches found
        longest_run = max(longest_run, count)

    # After checking for runs at each character in seqeuence, return longest run found
    return longest_run


main()
