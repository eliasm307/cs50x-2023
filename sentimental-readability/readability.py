import re

# get text from user
text = input("Text: ")

# count of alphabetical characters
letter_count = len(re.findall("[a-zA-Z]", text))

# count of sequenced of characters separated by a space
word_count = len(text.split(" "))

# count of sentence end characters
sentence_count = len(re.findall("[.!?]", text))

# calculate reading grade using Coleman-Liau formula
L = 100 * letter_count / word_count
S = 100 * sentence_count / word_count
grade = round((0.0588 * L) - (0.296 * S) - 15.8)

# print reading grade
if grade > 16:
    print("Grade 16+")

elif grade < 1:
    print("Before Grade 1")

else:
    print(f"Grade {grade}")
