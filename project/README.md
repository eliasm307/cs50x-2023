# CS50 Pokedex

#### Video Demo: https://www.youtube.com/watch?v=BIg__dlI5k8

#### Description:

Explore the realm of Pokémon with our web-based "Pokedex" application served from a Python/Flask server.

It serves as your digital guide, providing detailed information on Pokémon species, much like the iconic device in the Pokémon universe. Powered by the open-source [PokeAPI](https://pokeapi.co/), here's what you can do:

- **Search**: Find Pokémon by name quickly.
- **Discover**: Delve into profiles, uncovering stats and connections to other Pokémon.
- **Interact**: Register to personalize your experience, save favorites, and revisit them anytime.
- **Responsive Design**: Enjoy a seamless experience on devices of all sizes.

## Files

Below are the files used for this project and a general description of their purpose/function.

### static/fonts/

This is a folder which contains the custom fonts used for the site, which are PokemonHollow.ttf and PokemonXandY.ttf, which come from [dafont.com](https://www.dafont.com/).

Custom fonts were used to more closely emulate the Pokemon theme and make the site seem more familar compared to the Pokemon games.

### static/fonts/

This directory contains the custom font files `PokemonHollow.ttf` and `PokemonXandY.ttf`, obtained from [dafont.com](https://www.dafont.com/).

Custom fonts were used to mirror the thematic aesthetics of the Pokémon series, providing a familiar visual context for users. They are applied globally across the site's UI to maintain a consistent Pokémon-themed experience.


### static/media/

This is a folder which contains images used for the site, which are `open-pokeball.png` and `pokeball.png`, and which come from [copyicon.com](https://copyicon.com/).

The images are used as the favicon for all pages and as the menu button in the nav bar when on smaller screens, to fit the Pokemon theme.

### static/styles.css

This stylesheet is responsible for the custom styling of the entire web application.

It not only incorporates the custom fonts from the `static/fonts/` directory to align the site's visual theme with the Pokémon style, but it also provides specific CSS rules targeting and styling various page elements.

### templates/layout.html

This file defines the base layout template and HTML structure of all pages, including:
- the `<head>` element which includes setup links and scripts for bootstrap, page metadata, and links to custom CSS and scripts
- a nav bar which has conditional links depending on whether a user is signed in or not.
- a footer which includes some text about how the site is not an official Pokemon website and also lists some acknowledgements of where resources and information comes from.
- a global script which defines a function that allows Pokemon images/GIFs fallback to a default if error occurs or relevant image doesnt exist in open source PokeAPI sprites repository.

### templates/index.html

Template for the `/` route.

This defines a page which shows the list of all available Pokemon.

Every Pokemon is shown as a card which shows a GIF of the Pokemon, if available (the GIFs come from an open source un-official source so might be missing some depending on how well the repository is maintained) otherwise a default fallback image is shown.

Each Pokemon card is a hyperlink that opens the relevant Pokemon's overview page when clicked.

The page includes controls to filter the list to only those Pokemon with a name containing some given text.

As the list of Pokemon can be long, e.g. over 1000 Pokemon in total, the GIF's `<image>` elements are lazy loaded, and only the visible images are fetched to improve initial load and usability and minimise image resource requests.

### templates/overview.html

Template for the `/pokemon/<pokemon_id>` routes.

This defines an overview page of a Pokemon which shows various information about it such as:
- A GIF or image of the Pokemon
- General data e.g. weight, height, and the types of the Pokemon
- Descriptions related to the Pokemon and what games they come from
- The varieties of the Pokemon, according to the PokeAPI database
- Where applicable, the Pokemon it evolves from/to as cards with GIFs/images that are hyperlinks to those Pokemon's overview pages

In addition:
- When a user is signed in a button to add/remove the Pokemon to/from the users favourites is exposed, which are persisted to their account (ie favourites will still be there after logging out then logging in again).
- When this route is loaded, the page's header and footer are coloured based on the Pokemon's main colour, which is applied via a class added to the `<body>` element via `layout.html`.

### templates/register.html

Template for the `/register` route.

This defines a page with a form for registering a new user for an account with the website.

The route also includes some validation, such as:
- making sure relevant fields are filled in
- making sure a user cannot register using an existing user name

### templates/login.html

Template for the `/login` route.

This defines a page with a form for logging in an existing user.

The route also includes some validation such as:
- making sure relevant fields are filled in
- making sure user details match an existing user

### templates/profile.html

Template for the `/profile` route, which is only accessible to a user who is logged in.

The template defines a page with a form that a user can use to change their user name and password that they use for logging in.

The route also includes validation such as:
- making sure user name changes do not clash with another existing user

### templates/favourites.html

Template for the `/favourites` route, which is only accessible to a user who is logged in.

The template defines a page with a list of Pokemon the user has added to their favourites.

The Pokemon are shown as a list of cards which are hyperlinks to the relevant Pokemon overview pages and also include a button to remove the Pokemon from favourites.

The page includes controls to filter the list to only those Pokemon with a name containing some given text.

### app.py

This defines the Flask server which defines all routes that handle requests and also populate and serves the templates described above with relevant data and supports functionality for all pages.

This also handles user authentication and interfaces with an SQLite database to persist some information.

### helpers.py

This is a file of Python helpers used by `app.py` for functionality such as:
- requiring authentication for routes
- utilities for fetching data from `PokeAPI` and formatting the output data before being used to populate HTML templates
    - data fetching is done using the `pokebase` package, which is the official `PokeAPI` Python client and it is used to provide suitable caching to satisfy the "Fair Use Policy"
- utilities for creating a pokemon graphic URL from a given ID, which come from this [Github repository](https://github.com/PokeAPI/sprites)

### project.db

This is the SQLLite database where persistent data for the site is stored such as users and their favourite Pokemon.

The schema for the data base is:

```sql
CREATE TABLE sqlite_sequence(name,seq);
CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, name TEXT NOT NULL, hash TEXT NOT NULL);
CREATE UNIQUE INDEX user_name ON users (name);
CREATE TABLE favourites (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, user_id INTEGER NOT NULL, pokemon_id INTEGER NOT NULL);
CREATE INDEX favourite_user_id ON favourites (user_id);
```
