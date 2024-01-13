import cs50


def get_height_from_user():
    while True:
        height = cs50.get_int("Height: ")
        if height > 0 and height <= 8:
            return height


height = get_height_from_user()


for rowNumber in range(1, height + 1):
    print(
        " " * (height - rowNumber),  # left pyramid prefix spacing per row
        "#" * rowNumber,  # left pyramid blocks
        " " * 2,  # spacing between pyramids
        "#" * rowNumber,  # right pyramid blocks
        sep="",
    )
