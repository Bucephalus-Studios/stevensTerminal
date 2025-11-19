/**
 * stevensMathLib.h
 *
 * A modern C++ library containing useful functions for numerical operations,
 * random number generation, and range checking.
 *
 * Copyright (c) 2025 Bucephalus-Studios
 * Licensed under the MIT License
 */

#ifndef STEVENSMATHLIB_H
#define STEVENSMATHLIB_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace stevensMathLib
{
    /**
     * @brief Specifies whether range bounds are inclusive or exclusive
     */
    enum class BoundType
    {
        Inclusive,  // Uses >= and <= comparisons
        Exclusive   // Uses > and < comparisons
    };

    /**
     * @brief Seeds the legacy random number generator
     *
     * @deprecated Use getRandomEngine() for modern C++ random number generation
     *
     * This function seeds the C-style rand() function with the current time.
     * For new code, prefer using the modern C++ <random> library.
     */
    inline void seedRNG()
    {
        srand(static_cast<unsigned>(time(nullptr)));
    }

    /**
     * @brief Returns a thread-local random number engine
     *
     * @return A reference to a mt19937 random engine, seeded with random_device
     *
     * This provides a modern, high-quality random number generator using
     * the Mersenne Twister algorithm. The engine is thread-local to avoid
     * race conditions in multithreaded code.
     */
    inline std::mt19937& getRandomEngine()
    {
        thread_local std::random_device randomDevice;
        thread_local std::mt19937 engine(randomDevice());
        return engine;
    }

    /**
     * @brief Checks if a value has no fractional part
     *
     * @param value The value to check
     * @return true if the value equals its floor (no fractional part)
     */
    inline bool isWholeNumber(float value)
    {
        return std::floor(value) == value;
    }

    /**
     * @brief Rounds a number to the nearest tenth
     *
     * @param numberToRound The number to round
     * @return The number rounded to one decimal place, or as an integer if whole
     *
     * If the number has no decimal part (or rounds to a whole number),
     * returns it as an integer. Otherwise, rounds to the nearest tenth.
     */
    inline float roundToNearest10th(float numberToRound)
    {
        if (isWholeNumber(numberToRound)) {
            return static_cast<int>(numberToRound);
        }

        const float roundedValue = std::round(numberToRound * 10.0f) / 10.0f;

        if (isWholeNumber(roundedValue)) {
            return static_cast<int>(roundedValue);
        }

        return roundedValue;
    }

    /**
     * @brief Rounds a number to a specified precision
     *
     * @param value The value to round
     * @param decimalPlaces The number of decimal places to round to
     * @return The value rounded to the specified decimal places
     *
     * @note Precision must be non-negative. For precision 0, rounds to integer.
     *
     * Example: round(3.14159, 2) returns 3.14
     */
    inline float round(float value, int decimalPlaces)
    {
        if (decimalPlaces < 0) {
            std::cerr << "Warning: negative precision (" << decimalPlaces
                      << ") provided to round(). Using absolute value.\n";
            decimalPlaces = std::abs(decimalPlaces);
        }

        const float scaleFactor = std::pow(10.0f, decimalPlaces);
        return std::round(value * scaleFactor) / scaleFactor;
    }

    /**
     * @brief Generates a random floating point number in a range
     *
     * @param lowerBound Inclusive lower bound (minimum value)
     * @param upperBound Inclusive upper bound (maximum value)
     * @return A random float between lowerBound and upperBound
     *
     * Uses modern C++ random number generation for better distribution
     * quality compared to the legacy rand() function.
     */
    inline float randomFloat(float lowerBound = 0.0f, float upperBound = 1.0f)
    {
        std::uniform_real_distribution<float> distribution(lowerBound, upperBound);
        return distribution(getRandomEngine());
    }

    /**
     * @brief Generates a random integer in a range
     *
     * @param lowerBound Inclusive lower bound (minimum value)
     * @param upperBound Exclusive upper bound (one past maximum value)
     * @return A random integer in [lowerBound, upperBound)
     *
     * If upperBound <= lowerBound, returns lowerBound.
     * Uses modern C++ random number generation.
     */
    inline int randomInt(int lowerBound = 0, int upperBound = 100)
    {
        if (upperBound <= lowerBound) {
            return lowerBound;
        }

        std::uniform_int_distribution<int> distribution(lowerBound, upperBound - 1);
        return distribution(getRandomEngine());
    }

    /**
     * @brief Generates a random integer not in a blacklist
     *
     * @param blacklist Vector of integers to exclude
     * @param lowerBound Inclusive lower bound
     * @param upperBound Exclusive upper bound
     * @return A random integer in range that's not in the blacklist
     *
     * @throws std::invalid_argument if all integers in range are blacklisted
     *
     * This function uses a rejection sampling approach: it generates random
     * integers until finding one not in the blacklist. This is efficient
     * when the blacklist is small relative to the range.
     */
    inline int randomIntNotInBlacklist(const std::vector<int>& blacklist,
                                       int lowerBound = 0,
                                       int upperBound = 100)
    {
        const int rangeSize = upperBound - lowerBound;

        if (rangeSize <= 0) {
            throw std::invalid_argument(
                "stevensMathLib::randomIntNotInBlacklist - upperBound must be greater than lowerBound");
        }

        if (static_cast<int>(blacklist.size()) >= rangeSize) {
            throw std::invalid_argument(
                "stevensMathLib::randomIntNotInBlacklist - blacklist size >= available range");
        }

        // Rejection sampling: keep generating until we find a non-blacklisted value
        const int maxAttempts = rangeSize * 10;
        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            const int candidate = randomInt(lowerBound, upperBound);

            const bool isBlacklisted = std::find(blacklist.begin(), blacklist.end(),
                                                 candidate) != blacklist.end();
            if (!isBlacklisted) {
                return candidate;
            }
        }

        // Fallback: build valid set if rejection sampling fails
        std::vector<int> validNumbers;
        validNumbers.reserve(rangeSize - blacklist.size());

        for (int i = lowerBound; i < upperBound; ++i) {
            const bool isBlacklisted = std::find(blacklist.begin(), blacklist.end(),
                                                 i) != blacklist.end();
            if (!isBlacklisted) {
                validNumbers.push_back(i);
            }
        }

        if (validNumbers.empty()) {
            throw std::invalid_argument(
                "stevensMathLib::randomIntNotInBlacklist - no valid integers remain after blacklist");
        }

        const int randomIndex = randomInt(0, static_cast<int>(validNumbers.size()));
        return validNumbers[randomIndex];
    }

    /**
     * @brief Safely converts a floating point number to an integer
     *
     * @tparam FloatType The floating point type (float, double, long double)
     * @param value The floating point value to convert
     * @return The value converted to int, or int::max() if out of range
     *
     * This function checks if the float value fits within the int range
     * before conversion. If out of range, it prints a warning and returns
     * the maximum int value.
     */
    template <typename FloatType>
    int floatToInt(const FloatType& value)
    {
        const bool floatHasMorePrecision =
            std::numeric_limits<int>::digits < std::numeric_limits<FloatType>::digits;

        if (!floatHasMorePrecision) {
            return static_cast<int>(value);
        }

        const FloatType maxIntAsFloat = static_cast<FloatType>(std::numeric_limits<int>::max());
        const FloatType minIntAsFloat = static_cast<FloatType>(std::numeric_limits<int>::min());

        const bool fitsInInt = (value < maxIntAsFloat) && (value > minIntAsFloat);

        if (fitsInInt) {
            return static_cast<int>(value);
        }

        std::cerr << "Warning: unsafe conversion of value " << value
                  << " to int. Returning int::max().\n";
        return std::numeric_limits<int>::max();
    }

    /**
     * @brief Checks if a value falls within a specified range
     *
     * @tparam T The numeric type (must support comparison operators)
     * @param value The value to check
     * @param lowerBound The lower bound of the range
     * @param upperBound The upper bound of the range
     * @param boundType Whether bounds are inclusive or exclusive
     * @return true if value is within the range, false otherwise
     *
     * Example:
     *   in_range(5, 0, 10, BoundType::Inclusive)  -> true
     *   in_range(10, 0, 10, BoundType::Exclusive) -> false
     */
    template <typename T>
    bool in_range(const T& value,
                  const T& lowerBound,
                  const T& upperBound,
                  BoundType boundType = BoundType::Inclusive)
    {
        if (boundType == BoundType::Inclusive) {
            return (value >= lowerBound) && (value <= upperBound);
        }

        return (value > lowerBound) && (value < upperBound);
    }

} // namespace stevensMathLib

#endif // STEVENSMATHLIB_H
