from cs50 import SQL
from flask import Flask, flash, redirect, render_template, request, session
from flask_session import Session
from werkzeug.security import check_password_hash, generate_password_hash
from helpers import (
    login_required,
    get_pokemon_list,
    get_pokemon_overview_by_id,
    get_total_pokemon_count,
    get_favourite_pokemon_list,
)

# Configure application
app = Flask(__name__)

# Configure session to use filesystem (instead of signed cookies)
app.config["SESSION_PERMANENT"] = False
app.config["SESSION_TYPE"] = "filesystem"
Session(app)

# Configure CS50 Library to use SQLite database
db = SQL("sqlite:///project.db")


def is_logged_in():
    if not "user_id" in session:
        return False

    return bool(session["user_id"])


@app.after_request
def after_request(response):
    """Ensure responses aren't cached"""
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Expires"] = 0
    response.headers["Pragma"] = "no-cache"
    return response


@app.route("/register", methods=["GET", "POST"])
def register():
    template_filename = "register.html"

    if request.method == "POST":
        # get user name
        user_name = request.form.get("username")
        if not user_name:
            flash("Must provide user name")
            return render_template(template_filename), 400

        # check user name is unique
        existing_users = db.execute(
            """
            SELECT id
            FROM users
            WHERE name = ?
            LIMIT 1;
            """,
            user_name
        )
        if len(existing_users) > 0:
            flash("User name already exists")
            return render_template(template_filename), 400

        # get password
        password = request.form.get("password")
        if not password:
            flash("Must provide password")
            return render_template(template_filename), 400

        # get password confirmation
        password_confirmation = request.form.get("confirmation")
        if not password_confirmation:
            flash("Must provide password confirmation")
            return render_template(template_filename), 400

        # check password matches confirmation
        if password != password_confirmation:
            flash("Password does not match password confirmation")
            return render_template(template_filename), 400

        # Create user
        new_user_id = db.execute(
            """
            INSERT INTO users (name, hash)
            VALUES (?, ?);
            """,
            user_name,
            generate_password_hash(password),
        )

        # set user as logged in
        session["user_id"] = new_user_id

        # Redirect user to home page
        return redirect("/")

    # NOTE: assumes GET is the only other allowed method
    return render_template(template_filename)


@app.route("/login", methods=["GET", "POST"])
def login():
    template_filename = "login.html"

    # Forget any user_id
    session.clear()

    # User reached route via POST (as by submitting a form via POST)
    if request.method == "POST":
        user_name = request.form.get("username")
        password = request.form.get("password")

        # Ensure user name was submitted
        if not user_name:
            flash("Must provide user name")
            return render_template(template_filename), 403

        # Ensure password was submitted
        elif not password:
            flash("Must provide password")
            return render_template(template_filename), 403

        # Query database for user name
        rows = db.execute(
            """
            SELECT *
            FROM users
            WHERE name = ?;
            """,
            user_name
        )

        # Ensure user name exists and password is correct
        if len(rows) != 1 or not check_password_hash(rows[0]["hash"], password):
            flash("Invalid user name and/or password")
            return render_template(template_filename), 403

        # Remember which user has logged in
        session["user_id"] = rows[0]["id"]

        # Redirect user to home page
        return redirect("/")

    # User reached route via GET (as by clicking a link or via redirect)
    return render_template(template_filename)


@app.route("/logout")
def logout():
    # Forget any user_id ie logout user
    session.clear()

    # Redirect user to home page
    return redirect("/")


@app.route("/profile", methods=["GET", "POST"])
@login_required
def profile():
    template_filename = "profile.html"
    code = 200 # default assume all went well

    if request.method == "POST":
        new_user_name = request.form.get("username")
        new_assword = request.form.get("password")

        # check if no changes to apply
        if not new_user_name and not new_assword:
            flash("New user name or password must be provided")
            code = 400

        else:
            # check user name is not already owned by another user
            existing_users = db.execute(
                """
                SELECT id
                FROM users
                WHERE name = ? AND NOT id = ?
                LIMIT 1;
                """,
                new_user_name,
                session["user_id"]
            )

            if len(existing_users) > 0:
                flash("User name already taken")
                code = 400

            # check for user name and password update
            elif new_user_name and new_assword:
                db.execute(
                    """
                    UPDATE users
                    SET name = ?, hash = ?
                    WHERE id = ?;
                    """,
                    new_user_name,
                    generate_password_hash(new_assword),
                    session["user_id"],
                )
                flash("User name and password updated")

            # check for password only update
            elif new_assword:
                db.execute(
                    """
                    UPDATE users
                    SET hash = ?
                    WHERE id = ?;
                    """,
                    generate_password_hash(new_assword),
                    session["user_id"],
                )
                flash("Password updated")

            # otherwise user name only update
            else:
                db.execute(
                    """
                    UPDATE users
                    SET name = ?
                    WHERE id = ?;
                    """,
                    new_user_name,
                    session["user_id"],
                )
                flash("User name updated")

    # get latest user and render
    user = db.execute("SELECT name FROM users WHERE id = ?;", session["user_id"])[0]

    return render_template(template_filename, user=user), code


@app.route("/")
def index():
    """
    Any user can access this route to search and view pokemon
    """
    search_text = request.args.get("text")
    if not search_text:
        search_text = ""  # avoid using the string "None"

    # render page
    return render_template(
        "index.html",
        list=get_pokemon_list(filter_text=search_text),
        total_count=get_total_pokemon_count(),
        search_text=search_text,
        is_logged_in=is_logged_in(),
    )


REMOVE_FAVOURITE_ACTION = "Remove from Favourites"
ADD_FAVOURITE_ACTION = "Add to Favourites"



@app.route("/pokemon/<pokemon_id>", methods=["GET", "POST"])
def pokemon_overview(pokemon_id):
    """
    Any user can access this route to view pokemon

    Only authenticated users can add/remove pokemon to their favourites
    """

    pokemon_id = int(pokemon_id)  # make sure this is a number

    if request.method == "POST":
        action = request.form.get("action")
        if action == ADD_FAVOURITE_ACTION:
            db.execute(
                """
                INSERT INTO favourites (user_id, pokemon_id)
                VALUES (?, ?);
                """,
                session["user_id"],
                pokemon_id,
            )
            flash("Added to favourites")

        elif action == REMOVE_FAVOURITE_ACTION:
            db.execute(
                """
                DELETE FROM favourites
                WHERE user_id = ? AND pokemon_id = ?;
                """,
                session["user_id"],
                pokemon_id,
            )
            flash("Removed from favourites")

    favourite_button_action = None
    if is_logged_in():
        rows = db.execute(
            """
            SELECT id
            FROM favourites
            WHERE user_id = ? AND pokemon_id = ?;
            """,
            session["user_id"],
            pokemon_id,
        )

        if len(rows):
            favourite_button_action = REMOVE_FAVOURITE_ACTION
        else:
            favourite_button_action = ADD_FAVOURITE_ACTION

    # render page
    # NOTE: sets header and footer colour based on pokemon colour, see https://pokeapi.co/api/v2/pokemon-color
    pokemon = get_pokemon_overview_by_id(pokemon_id)
    return render_template(
        "overview.html",
        pokemon=pokemon,
        favourite_button_action=favourite_button_action,
        theme_colour=pokemon["colour"],
    )


@app.route("/favourites", methods=["GET", "POST"])
@login_required
def favourties():
    if request.method == "POST":
        action = request.form.get("action")
        pokemon_id = int(request.form.get("pokemon_id"))
        if action == REMOVE_FAVOURITE_ACTION:
            db.execute(
                """
                DELETE FROM favourites
                WHERE user_id = ? AND pokemon_id = ?;
                """,
                session["user_id"],
                pokemon_id,
            )

    search_text = request.args.get("text")
    if not search_text:
        search_text = ""  # avoid using the string "None"

    favourite_rows = db.execute(
        """
        SELECT *
        FROM favourites
        WHERE user_id = ?;
        """,
        session["user_id"],
    )

    return render_template(
        "favourites.html",
        list=get_favourite_pokemon_list(
            filter_text=search_text, favourite_rows=favourite_rows
        ),
        search_text=search_text,
        total_count=len(favourite_rows),
        favourite_button_action=REMOVE_FAVOURITE_ACTION,
    )
