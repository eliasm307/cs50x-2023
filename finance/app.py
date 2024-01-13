import os

from cs50 import SQL
from flask import Flask, flash, redirect, render_template, request, session
from flask_session import Session
from werkzeug.security import check_password_hash, generate_password_hash

from helpers import apology, login_required, lookup, usd

# Configure application
app = Flask(__name__)

# Custom filter
app.jinja_env.filters["usd"] = usd

# Configure session to use filesystem (instead of signed cookies)
app.config["SESSION_PERMANENT"] = False
app.config["SESSION_TYPE"] = "filesystem"
Session(app)

# Configure CS50 Library to use SQLite database
db = SQL("sqlite:///finance.db")


@app.after_request
def after_request(response):
    """Ensure responses aren't cached"""
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Expires"] = 0
    response.headers["Pragma"] = "no-cache"
    return response


@app.route("/")
@login_required
def index():
    """Show portfolio of stocks"""

    # get current balance
    user = db.execute("SELECT cash FROM users WHERE id = ?", session["user_id"])[0]
    current_balance = user["cash"]

    # get summary of transactions
    rows = db.execute(
        """
        SELECT symbol, SUM(shares_count) as count
        FROM transactions
        WHERE userid = ?
        GROUP BY symbol
        HAVING SUM(shares_count) > 0;
        """,
        session["user_id"],
    )

    # add total value per stock symbol and calulate total value of stocks
    stocks_total_value = 0
    for row in rows:
        stock = lookup(row["symbol"])
        row["share_price"] = stock["price"]
        row["total_value"] = row["count"] * stock["price"]
        stocks_total_value += row["total_value"]

    # get formatted grand total value
    grand_total = usd(current_balance + stocks_total_value)

    # format currency values
    current_balance = usd(current_balance)
    stocks_total_value = usd(stocks_total_value)
    index = 1
    for row in rows:
        row["index"] = index
        row["total_value"] = usd(row["total_value"])
        row["share_price"] = usd(row["share_price"])
        index += 1

    # render page
    return render_template(
        "index.html",
        rows=rows,
        current_balance=current_balance,
        stocks_total_value=stocks_total_value,
        grand_total=grand_total,
    )


@app.route("/buy", methods=["GET", "POST"])
@login_required
def buy():
    """Buy shares of stock"""

    if request.method == "POST":
        # get symbol from user
        symbol = request.form.get("symbol")
        if not symbol:
            return apology("must provide symbol", 400)

        symbol = symbol.upper()  # normalise to upper case symbols

        # get stock
        stock = lookup(symbol)
        if stock == None:
            return apology("symbol does not exist", 400)

        # get number of shares to buy from user
        try:
            shares_count = int(request.form.get("shares"))
            if shares_count <= 0:
                return apology("shares count must be a positive integer", 400)

        except ValueError:
            return apology("shares count must be a positive integer", 400)

        # check if user can afford transaction
        rows = db.execute("SELECT cash FROM users WHERE id = ?", session["user_id"])
        total_cost = stock["price"] * shares_count
        remaining_cash = rows[0]["cash"] - total_cost
        if remaining_cash < 0:
            return apology("insufficient funds for purchase", 400)

        # execute buy transaction
        db.execute(
            "UPDATE users SET cash = ? WHERE id = ?;",
            remaining_cash,
            session["user_id"],
        )
        db.execute(
            """
            INSERT INTO transactions (userid, symbol, shares_count, type, share_price, total_value)
            VALUES (?, ?, ?, ?, ?, ?);
            """,
            session["user_id"],
            symbol,
            shares_count,
            "BUY",
            stock["price"],
            -total_cost,  # NOTE: cost is negative here, as buying stocks reduces the amount of available cash
        )

        # redirect to home page
        return redirect("/")

    # refresh buy page
    return render_template("buy.html")


@app.route("/history")
@login_required
def history():
    """Show history of transactions"""

    # get summary of transactions
    # NOTE: shows most recent transactions first
    rows = db.execute(
        """
        SELECT *
        FROM transactions
        WHERE userid = ?
        ORDER BY timestamp DESC;
        """,
        session["user_id"],
    )

    # format currency values
    index = 1
    for row in rows:
        row["index"] = index
        row["share_price"] = usd(row["share_price"])
        row["total_value"] = usd(row["total_value"])
        index += 1

    # render page
    return render_template("history.html", rows=rows)


@app.route("/login", methods=["GET", "POST"])
def login():
    """Log user in"""

    # Forget any user_id
    session.clear()

    # User reached route via POST (as by submitting a form via POST)
    if request.method == "POST":
        # Ensure username was submitted
        if not request.form.get("username"):
            return apology("must provide username", 403)

        # Ensure password was submitted
        elif not request.form.get("password"):
            return apology("must provide password", 403)

        # Query database for username
        rows = db.execute(
            "SELECT * FROM users WHERE username = ?", request.form.get("username")
        )

        # Ensure username exists and password is correct
        if len(rows) != 1 or not check_password_hash(
            rows[0]["hash"], request.form.get("password")
        ):
            return apology("invalid username and/or password", 403)

        # Remember which user has logged in
        session["user_id"] = rows[0]["id"]

        # Redirect user to home page
        return redirect("/")

    # User reached route via GET (as by clicking a link or via redirect)
    else:
        return render_template("login.html")


@app.route("/logout")
def logout():
    """Log user out"""

    # Forget any user_id
    session.clear()

    # Redirect user to login form
    return redirect("/")


@app.route("/quote", methods=["GET", "POST"])
@login_required
def quote():
    """Get stock quote."""

    if request.method == "POST":
        # get symbol from user
        symbol = request.form.get("symbol")
        if not symbol:
            return apology("must provide symbol", 400)

        symbol = symbol.upper()  # normalise to upper case symbols

        # get related stock data
        stock = lookup(symbol)
        if stock == None:
            return apology("symbol is not valid", 400)

        # render quote
        return render_template(
            "quote.html", message=stock["name"] + " price is " + usd(stock["price"])
        )

    # handle GET request
    # NOTE: assumes GET is the only other allowed method
    return render_template("quote.html")


@app.route("/register", methods=["GET", "POST"])
def register():
    if request.method == "POST":
        # get username
        username = request.form.get("username")
        if not username:
            return apology("must provide username", 400)

        # check username is unique
        existing_users = db.execute(
            "SELECT id FROM users WHERE username = ? LIMIT 1;", username
        )
        if len(existing_users) > 0:
            return apology("username already exists", 400)

        # get password
        password = request.form.get("password")
        if not password:
            return apology("must provide password", 400)

        # get password confirmation
        password_confirmation = request.form.get("confirmation")
        if not password_confirmation:
            return apology("must provide password confirmation", 400)

        # check password matches confirmation
        if password != password_confirmation:
            return apology("password does not match password confirmation", 400)

        # Create user
        new_user_id = db.execute(
            "INSERT INTO users (username, hash) VALUES (?, ?);",
            username,
            generate_password_hash(password),
        )

        # set user as logged in
        session["user_id"] = new_user_id

        # Redirect user to home page
        return redirect("/")

    # NOTE: assumes GET is the only other allowed method
    return render_template("register.html")


@app.route("/sell", methods=["GET", "POST"])
@login_required
def sell():
    """Sell shares of stock"""

    if request.method == "POST":
        # get symbol from user
        symbol = request.form.get("symbol")
        if not symbol:
            return apology("must provide symbol", 400)

        symbol = symbol.upper()  # normalise to upper case symbols

        # get number of shares to sell from user
        try:
            sell_shares_count = int(request.form.get("shares"))
            if sell_shares_count <= 0:
                return apology("shares count must be a positive integer", 400)

        except ValueError:
            return apology("shares count must be a positive integer", 400)

        # get related stock data
        stock = lookup(symbol)
        if stock == None:
            return apology("invalid symbol", 400)

        # get details on how much of the stock the user holds
        rows = db.execute(
            """
            SELECT symbol, SUM(shares_count) as count
            FROM transactions
            WHERE userid = ? AND symbol = ?
            GROUP BY symbol
            HAVING SUM(shares_count) > 0;
            """,
            session["user_id"],
            symbol,
        )

        # check user owns stock to sell
        if len(rows) == 0:
            return apology("user does not own shares", 400)

        # check if user has enough shares to do transaction
        owned_stock = rows[0]
        remaining_shares_count = owned_stock["count"] - sell_shares_count
        if remaining_shares_count < 0:
            return apology("insufficient shares", 400)

        # execute sell transaction
        total_sell_value = sell_shares_count * stock["price"]
        current_cash = db.execute(
            "SELECT cash FROM users WHERE id = ?;", session["user_id"]
        )[0]["cash"]
        db.execute(
            "UPDATE users SET cash = ? WHERE id = ?;",
            current_cash + total_sell_value,
            session["user_id"],
        )
        db.execute(
            """
            INSERT INTO transactions (userid, symbol, shares_count, type, share_price, total_value)
            VALUES (?, ?, ?, ?, ?, ?);
            """,
            session["user_id"],
            symbol,
            -sell_shares_count,  # NOTE: negative as selling reduces the available stocks
            "SELL",
            stock["price"],
            total_sell_value,
        )

        # Redirect user to home page
        return redirect("/")

    # handle GET
    rows = db.execute(
        """
        SELECT symbol
        FROM transactions
        WHERE userid = ?
        GROUP BY symbol
        HAVING SUM(shares_count) > 0;
        """,
        session["user_id"],
    )
    return render_template("sell.html", stocks=rows)


@app.route("/account", methods=["GET", "POST"])
@login_required
def account():
    message = ""

    if request.method == "POST":
        # get form values
        new_username = request.form.get("username")
        new_assword = request.form.get("password")

        if not new_username and not new_assword:
            return redirect("/account")  # no changes to apply, ignore

        # Create user and set as logged in
        if new_assword:
            db.execute(
                "UPDATE users SET username = ?, hash = ? WHERE id = ?;",
                new_username,
                generate_password_hash(new_assword),
                session["user_id"],
            )

        else:
            db.execute(
                "UPDATE users SET username = ? WHERE id = ?;",
                new_username,
                session["user_id"],
            )

        message = "Changes applied!"

    # NOTE: assumes GET is the only other allowed method
    user = db.execute(
        "SELECT username, cash FROM users WHERE id = ?;", session["user_id"]
    )[0]
    user["cash"] = usd(user["cash"])
    return render_template("account.html", user=user, message=message)


@app.route("/deposit", methods=["POST"])
@login_required
def deposit():
    # get deposit amount from user
    try:
        deposit = float(request.form.get("deposit"))
        if deposit <= 0:
            return apology("Deposit must be a positive number", 400)

    except ValueError:
        return apology("Deposit must be a positive number", 400)

    # execute deposit
    user = db.execute("SELECT cash FROM users WHERE id = ?;", session["user_id"])[0]
    new_cash = user["cash"] + deposit
    db.execute("UPDATE users SET cash = ? WHERE id = ?;", new_cash, session["user_id"])

    # refresh accounts page
    return redirect("/account")
