const pickRandomButton = document.getElementById("pick-random");
pickRandomButton?.addEventListener("click", () => {
    const currentPokemonName = window.location.pathname.replace("/", "");
    const POKEMON_NAMES = ["bulbasaur", "charmander", "squirtle"];
    const possibleTargetNames = POKEMON_NAMES.filter(name => name !== currentPokemonName);
    const randomName = possibleTargetNames[Math.floor(Math.random() * possibleTargetNames.length)];
    window.location.href = `/${randomName}`;
})
