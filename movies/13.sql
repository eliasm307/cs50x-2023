SELECT DISTINCT people.name
FROM people
JOIN stars
    ON people.id = stars.person_id
WHERE stars.movie_id in (
        SELECT movies.id
        FROM movies
        JOIN stars
            ON movies.id = stars.movie_id
        JOIN people
            ON stars.person_id = people.id
        WHERE people.name = 'Kevin Bacon'
            AND people.birth = 1958
    )
    AND NOT (
        people.name = 'Kevin Bacon'
        AND people.birth = 1958
    );

