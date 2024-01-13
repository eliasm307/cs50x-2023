import os

from cs50 import SQL
from flask import Flask, flash, jsonify, redirect, render_template, request, session

# Configure application
app = Flask(__name__)

# Ensure templates are auto-reloaded
app.config["TEMPLATES_AUTO_RELOAD"] = True

# Configure CS50 Library to use SQLite database
db = SQL("sqlite:///birthdays.db")


@app.after_request
def after_request(response):
    """Ensure responses aren't cached"""
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Expires"] = 0
    response.headers["Pragma"] = "no-cache"
    return response


def validate_input_and_add_birthday(name, month, day):
    # Get user input and validate
    try:
        name = request.form.get("name")
        month = int(request.form.get("month"))
        if month < 1 or month > 12:
            return # Month is invalid

        day = int(request.form.get("day"))
        if day < 1 or day > 31:
            return # Day is invalid

    except:
        print("Validation failed")

    # Add the user's entry into the database
    else:
        db.execute("INSERT INTO birthdays (name, month, day) VALUES (?, ?, ?);", name, month, day)


def validate_input_and_delete_birthday(id):
    try:
        id = int(request.form.get("id"))

    except:
        print("Validation failed")

    # Add the user's entry into the database
    else:
        db.execute("DELETE FROM birthdays WHERE id=?;", id)


@app.route("/", methods=["GET", "POST"])
def index():
    if request.method == "POST":
        if request.form.get("type") == "delete":
            validate_input_and_delete_birthday(id=request.form.get("id"))

        else:
            validate_input_and_add_birthday(
                name=request.form.get("name"),
                month=request.form.get("month"),
                day=request.form.get("day")
            )

        # Refresh page in any case
        return redirect("/")

    else:
        # Display the entries in the database on index.html
        birthdays = db.execute("SELECT * FROM birthdays;")
        return render_template("index.html", birthdays=birthdays)


