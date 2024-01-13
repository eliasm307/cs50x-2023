from functools import wraps
from flask import redirect, session
import pokebase  # NOTE: using official library as this handles local caching, a requirement of the PokeAPI Fair Use Policy


def login_required(f):
    """
    Decorate routes to require login.

    http://flask.pocoo.org/docs/0.12/patterns/viewdecorators/
    """

    @wraps(f)
    def decorated_function(*args, **kwargs):
        if session.get("user_id") is None:
            return redirect("/login")
        return f(*args, **kwargs)

    return decorated_function


# example: https://pokeapi.co/api/v2/pokemon/7/
def get_id_from_url(url):
    return int(url.split("/")[-2])


def get_pokemon_graphic_url_from_id(id):
    id = int(id)  # will throw if id is not a number
    if id < 1:
        return ""

    # from https://github.com/PokeAPI/sprites
    return (
        "https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/other/showdown/"
        + str(id)
        + ".gif"
    )


def capitalize(string):
    return string[0].upper() + string[1:]


def get_pokemon_list(filter_text):
    output = []
    # NOTE: some pokemon are not linked to a species, so this makes sure we only show those that have a related species (ie instead of listing pokemon directly)
    for pokemon in iter(pokebase.APIResourceList("pokemon-species")):
        # see pokemon data shape here: https://pokeapi.co/docs/v2#pokemon
        if not filter_text or filter_text.lower() in pokemon["name"]:
            id = get_id_from_url(pokemon["url"])
            output.append(
                {
                    "id": id,
                    # NOTE: the raw name is all lower case, we assume capitalising it produces the english display name
                    "name": capitalize(pokemon["name"]),
                    "graphic_url": get_pokemon_graphic_url_from_id(id),
                }
            )

    return output


pokemon_id_to_name_cache = {}


def get_raw_pokemon_name_from_id(id):
    # load cache if not done already
    id = int(id)
    if not id in pokemon_id_to_name_cache:
        for pokemon in iter(pokebase.APIResourceList("pokemon-species")):
            cache_id = get_id_from_url(pokemon["url"])
            pokemon_id_to_name_cache[cache_id] = pokemon["name"]

    # return cached name
    return pokemon_id_to_name_cache[id]


def get_favourite_pokemon_list(filter_text, favourite_rows):
    output = []
    for row in favourite_rows:
        id = row["pokemon_id"]
        raw_name = get_raw_pokemon_name_from_id(id)
        if not filter_text.lower() in raw_name:
            continue

        output.append(
            {
                "id": id,
                # NOTE: the raw name is all lower case, we assume capitalising it produces the english display name
                "name": capitalize(raw_name),
                "graphic_url": get_pokemon_graphic_url_from_id(id),
            }
        )

    return output





def get_pokemon_graphic_url(pokemon):
    """
    Example "sprites" property shape (url values can be defined or null):

    "sprites": {
            "back_default": null,
            "back_female": null,
            "back_shiny": null,
            "back_shiny_female": null,
            "front_default": "https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/929.png",
            "front_female": null,
            "front_shiny": null,
            "front_shiny_female": null,
            "other": {
                "dream_world": {
                    "front_default": null,
                    "front_female": null
                },
                "home": {
                    "front_default": null,
                    "front_female": null,
                    "front_shiny": null,
                    "front_shiny_female": null
                },
                "official-artwork": {
                    "front_default": "https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/other/official-artwork/929.png",
                    "front_shiny": "https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/other/official-artwork/shiny/929.png"
                },
                "showdown": {
                    "back_default": null,
                    "back_female": null,
                    "back_shiny": null,
                    "back_shiny_female": null,
                    "front_default": null,
                    "front_female": null,
                    "front_shiny": null,
                    "front_shiny_female": null
                }
            }
        }
    """
    sprites = pokemon.sprites
    if sprites.other.showdown.front_default:
        return sprites.other.showdown.front_default

    elif sprites.other.official_artwork.front_default:
        return sprites.other.official_artwork.front_default

    elif sprites.front_default:
        return sprites.front_default

    return FALLBACK_GRAPHIC_URL


version_name_cache = {}


# NOTE: this is slow for the first requests before cache is filled
# NOTE: not storing this in SQL database as we would then need to manage syncronisation etc,
#       this way we get the latest version names and cache in memory,
#       which should last as long as the service is active, just the first request will suffer
def get_version_display_name(raw_name):
    if not raw_name in version_name_cache:
        version = pokebase.version(raw_name)
        for name in version.names:
            if name.language.name == "en":
                version_name_cache[raw_name] = name.name
                break

    return version_name_cache[raw_name]


def get_pokemon_species_flavour_entries(species):
    text_to_versions_map = {}

    # collect texts and versions they appear in
    for entry in species.flavor_text_entries:
        if entry.language.name != "en":
            continue

        # the raw version names here are more like IDs, fetch the version diplay names instead
        version_display_name = get_version_display_name(entry.version.name)

        text = entry.flavor_text.replace("\n", " ").replace("\f", " ").strip()
        text_key = text.lower()  # sometimes the difference is just casing
        if not text_key in text_to_versions_map:
            text_to_versions_map[text_key] = {
                "text": text,
                "game_versions": [version_display_name],
            }

        else:
            text_to_versions_map[text_key]["game_versions"].append(version_display_name)

    # format output
    output = []
    for text in text_to_versions_map:
        output.append(text_to_versions_map[text])

    return output


def get_pokemon_species_display_name(species):
    for entry in species.names:
        """
        Example entry:
        {
            "language": {
                "name": "ja-Hrkt",
                "url": "https://pokeapi.co/api/v2/language/1/"
            },
            "name": "カメックス"
        },
        """
        if entry.language.name == "en":
            return entry.name

    return species.name  # fallback


def get_pokemon_species_colour(species):
    return species.color.name


def get_pokemon_species_varieties(species, current_pokemon_id):
    output = []
    for variety in iter(species.varieties):
        pokemon = variety.pokemon
        id = get_id_from_url(pokemon.url)
        if id == current_pokemon_id:
            continue  # dont show current pokemon as its own variety

        output.append(
            {
                "id": id,
                # NOTE: the raw name is all lower case, we assume capitalising it produces the english display name
                "name": capitalize(pokemon.name),
                "graphic_url": get_pokemon_graphic_url_from_id(id),
            }
        )

    return output


def get_evolves_from_species(species):
    evolves_from = species.evolves_from_species
    if not evolves_from:
        return

    id = get_id_from_url(evolves_from.url)
    return {
        "name": capitalize(evolves_from.name),
        "id": id,
        "graphic_url": get_pokemon_graphic_url_from_id(id),
    }


def find_evolution_chain_link_by_id(evolution_chain_link, target_id):
    current_id = get_id_from_url(evolution_chain_link.species.url)
    if current_id == target_id:
        return evolution_chain_link

    for next_evolution_chain_link in evolution_chain_link.evolves_to:
        found = find_evolution_chain_link_by_id(next_evolution_chain_link, target_id)
        if found:
            return found


def get_evolves_to_species(species, current_pokemon_id):
    """
    Example evolution chain link:
    {
        "evolution_details": [{...}],
        "evolves_to": [
            {
                "evolution_details": [{...}],
                "evolves_to": [{...}],
                "is_baby": false,
                "species": {
                    "name": "charizard",
                    "url": "https://pokeapi.co/api/v2/pokemon-species/6/"
                }
            }
        ],
        "is_baby": false,
        "species": {
            "name": "charmeleon",
            "url": "https://pokeapi.co/api/v2/pokemon-species/5/"
        }
    }
    """
    first_evolution_chain_link = species.evolution_chain.chain

    target_evolution_chain_link = find_evolution_chain_link_by_id(
        target_id=current_pokemon_id, evolution_chain_link=first_evolution_chain_link
    )

    output = []

    if target_evolution_chain_link:
        for next_evolution in target_evolution_chain_link.evolves_to:
            id = get_id_from_url(next_evolution.species.url)
            output.append(
                {
                    "name": capitalize(next_evolution.species.name),
                    "id": id,
                    "graphic_url": get_pokemon_graphic_url_from_id(id),
                }
            )

    return output


def get_pokemon_overview_by_id(pokemon_id):
    pokemon_id = int(pokemon_id)  # also ensures id is a number

    # see data shape here: https://pokeapi.co/docs/v2#pokemon
    pokemon = pokebase.pokemon(pokemon_id)
    # see data shape here: https://pokeapi.co/docs/v2#pokemon-species
    pokemon_species = pokebase.pokemon_species(pokemon_id)

    return {
        "types": pokemon.types,
        "graphic_url": get_pokemon_graphic_url(pokemon),
        "display_name": get_pokemon_species_display_name(pokemon_species),
        "flavour_entries": get_pokemon_species_flavour_entries(pokemon_species),
        "colour": get_pokemon_species_colour(pokemon_species),
        "weight": str(pokemon.weight / 10) + "kg",  # raw weight in hectograms
        "height": str(pokemon.height / 10) + "m",  # raw height in decimetres
        "varieties": get_pokemon_species_varieties(
            species=pokemon_species, current_pokemon_id=pokemon_id
        ),
        "evolves_from": get_evolves_from_species(pokemon_species),
        "evolves_to": get_evolves_to_species(
            species=pokemon_species, current_pokemon_id=pokemon_id
        ),
    }


def get_total_pokemon_count():
    # NOTE: only handling pokemon with species, that we can show information for
    return len(pokebase.APIResourceList("pokemon-species"))
