-- Keep a log of any SQL queries you execute as you solve the mystery.

-- viewing reports from the day of the crime
-- the theft took place on July 28, 2021
-- on Humphrey Street
-- NOTE: assuming month and day start from 1, ie month 1 is January and 1 is the first day of the month,
--       since there are no 0 values in these columns, also there is a day row with 31,
--       ie the max day when based on 1 which is not possible if it was based on 1.
--       Assuming months format is consistent with days

SELECT *
FROM crime_scene_reports
WHERE year = 2021
    AND month = 7
    AND day = 28
    AND street = 'Humphrey Street';

-----------------------------------------------------------------------------------
-- From the query above, found the report for the CS50 duck theft, which happened at 10:15am. The report mentions:
-- "Interviews were conducted today with three witnesses who were present at the time – each of their interview transcripts mentions the bakery."
-- so going to check out the interviews on this day which mention the bakery (assuming date format is the same for all tables).

SELECT *
FROM interviews
WHERE year = 2021
    AND month = 7
    AND day = 28
    AND transcript LIKE '%bakery%';

-- Interview 1 transcript from Ruth:
--      Sometime within ten minutes of the theft, I saw the thief get into a car in the bakery parking lot and drive away.
--      If you have security footage from the bakery parking lot, you might want to look for cars that left the parking lot in that time frame.

-- Interview 2 transcript from Eugene:
--      I don't know the thief's name, but it was someone I recognized. Earlier this morning, before I arrived at Emma's bakery,
--      I was walking by the ATM on Leggett Street and saw the thief there withdrawing some money.

-- Interview 3 transcript from Raymond:
--      As the thief was leaving the bakery, they called someone who talked to them for less than a minute.
--      In the call, I heard the thief say that they were planning to take the earliest flight out of Fiftyville tomorrow.
--      The thief then asked the person on the other end of the phone to purchase the flight ticket.

-----------------------------------------------------------------------------------
-- From the interviews I am following up on the clues:
-- NOTE: Eugene recognised the person

-----------------------------------------------------------------------------------
-- - The thief was seen leaving in a car, checking bakery_security_logs table around 10mins after the time of the theft (ie after 10:15am):
SELECT *
FROM bakery_security_logs
WHERE year = 2021
    AND month = 7
    AND day = 28
    AND hour = 10
    AND minute > 15
    AND minute < 25
    AND activity = 'exit';

-- NOTE: various cars left in this time, this is not conclusive but we have a list of potential vehicles for the thief.

-----------------------------------------------------------------------------------
-- - The thief withdrew money on Leggett Street, checking atm_transactions table for this location on the day of the theft before the time of the crime, 10:15am:
SELECT *
FROM atm_transactions
WHERE year = 2021
    AND month = 7
    AND day = 28
    AND atm_location = 'Leggett Street'
    AND transaction_type = 'withdraw';

-- NOTE: various withdrawals were made on this day in that location, this is not conclusive but we have a list of potential account numbers for the thief.

-----------------------------------------------------------------------------------
-- - Thief made a call to their accomplice after leaving the bakery, will check phone_calls table for calls on this day for less than a minute
-- NOTE: assumes duration is in seconds

SELECT *
FROM phone_calls
WHERE year = 2021
    AND month = 7
    AND day = 28
    AND duration < 60;

-- NOTE: various calls were made on this day for less than a minute, this is not conclusive but we have a list of potential phone numbers for the thief and their accomplice.

-----------------------------------------------------------------------------------
-- - Thief plans to take earliest flight out of town on the next day, ie July 29, 2021, will find out what is the earliest flight on that day in the flights table
-- - NOTE: Thief's accomplice bought a plane ticket on the day of the crime at the same time of the short call, dont have information to check details of this, but thief has accomplice

SELECT *
FROM flights
JOIN airports
    ON airports.id = flights.origin_airport_id
WHERE year = 2021
    AND month = 7
    AND day = 29
    AND airports.city = 'Fiftyville'
ORDER BY hour ASC,
    minute ASC
LIMIT 1;

-- the earliest flight is at 8:20 am from Fiftyville (origin_airport_id 8) to destination_airport_id 4, which is:

SELECT *
FROM airports
WHERE id = 4;

-- LaGuardia Airport (LGA) in New York City

-----------------------------------------------------------------------------------
- We know the flight the thief plans on taking so I'll check the passenger table and find passengers of that flight and
- cross-reference the passport number with the people table then filter for a list of suspects, ie for people with:
- - a car licence number matching one that was leaving the scene of the crime
- - a phone number that made a call after the crime
- - an account_number matching one that made a withdrawal on the day of the crime

SELECT people.id, people.name, people.phone_number
FROM people
JOIN passengers
    ON passengers.passport_number = people.passport_number
JOIN flights
    ON flights.id = passengers.flight_id
JOIN bank_accounts
    ON bank_accounts.person_id = people.id
WHERE flights.origin_airport_id = 8
    AND flights.destination_airport_id = 4
    AND people.license_plate IN (
        SELECT license_plate
        FROM bakery_security_logs
        WHERE year = 2021
            AND month = 7
            AND day = 28
            AND hour = 10
            AND minute > 15
            AND minute < 25
            AND activity = 'exit'
    )
    AND people.phone_number IN (
        SELECT caller
        FROM phone_calls
        WHERE year = 2021
            AND month = 7
            AND day = 28
            AND duration < 60
    )
    AND bank_accounts.account_number IN (
        SELECT account_number
        FROM atm_transactions
        WHERE year = 2021
            AND month = 7
            AND day = 28
            AND atm_location = 'Leggett Street'
            AND transaction_type = 'withdraw'
    );

-- This produces a single person, Bruce (with id 686048 and phone number '(367) 555-5533'), who is the thief!

-----------------------------------------------------------------------------------
-- We can find the accomplice as the person Bruce called after the crime:

SELECT people.id, people.name
FROM people
WHERE people.phone_number IN (
        SELECT receiver
        FROM phone_calls
        WHERE year = 2021
            AND month = 7
            AND day = 28
            AND duration < 60
            AND caller = '(367) 555-5533'
    );

-- This produces a single person, Robin (with id 864400)


-----------------------------------------------------------------------------------
-- Summary
-- - The thief is Bruce (with id 686048)
-- - The thief escaped to LaGuardia Airport (LGA) in New York City
-- - The thief's accomplice is Robin (with id 864400)
