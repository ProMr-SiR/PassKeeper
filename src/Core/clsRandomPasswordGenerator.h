#pragma once
#include <Arduino.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * @class clsRandomPasswordGenerator
 * @brief Utility class for generating secure, random passwords.
 * Uses the hardware RNG (Random Number Generator) of the ESP32 (esp_random).
 * Provides features for creating complex passwords and omitting ambiguous characters.
 */
class clsRandomPasswordGenerator {
public:

    // -------------------------------------------------------------
    // Enum defining the type of characters to include in generation
    // -------------------------------------------------------------
    enum enCharType {
        SmallLetter        = 1,  // a-z
        CapitalLetter      = 2,  // A-Z
        Digit              = 3,  // 0-9
        SpecialCharacter   = 4,  // Special chars like @, #, etc.
        MixChars           = 5,  // Mix of Letters and Digits
        MixCharsAndSpecial = 6   // Mix of Letters, Digits, and Special Chars
    };

    /**
     * @brief Generates a true random number between From and To using ESP32 hardware RNG.
     */
    static int RandomNumber(int From, int To) {
        if (From >= To) return From;
        // Use esp_random() for true hardware-based entropy
        return (int)(esp_random() % (uint32_t)(To - From + 1)) + From;
    }
    
    /**
     * @brief Generates a single random character based on the requested character type.
     * @param CharType The type of character to generate.
     * @param specialSet Optional custom set of special characters.
     * @param omitAmbiguous If true, ambiguous characters like l, 1, O, 0 are omitted.
     * @return char The generated character.
     */
    static char GetRandomCharacter(enCharType CharType,
                                   const string &specialSet = "!@#$%^&*_-+=",
                                   bool omitAmbiguous = false) {
        
        // Randomize the character type if mix is selected
        if (CharType == MixChars)
            CharType = (enCharType)RandomNumber(SmallLetter, Digit);
        else if (CharType == MixCharsAndSpecial)
            CharType = (enCharType)RandomNumber(SmallLetter, SpecialCharacter);

        switch (CharType) {
            case SmallLetter: {
                if (omitAmbiguous) {
                    // Omit lower-case 'l' (L) and 'o' (O)
                    const string pool = "abcdefghijkmnopqrstuvwxyz";
                    return pool[RandomNumber(0, (int)pool.size() - 1)];
                }
                return (char)RandomNumber('a', 'z');
            }
            case CapitalLetter: {
                if (omitAmbiguous) {
                    // Omit upper-case 'I' (i) and 'O' (o)
                    const string pool = "ABCDEFGHJKLMNPQRSTUVWXYZ";
                    return pool[RandomNumber(0, (int)pool.size() - 1)];
                }
                return (char)RandomNumber('A', 'Z');
            }
            case Digit: {
                if (omitAmbiguous) {
                    // Omit '0' and '1'
                    const string pool = "23456789";
                    return pool[RandomNumber(0, (int)pool.size() - 1)];
                }
                return (char)RandomNumber('0', '9');
            }
            case SpecialCharacter: {
                // Use default or provided special characters set
                const string &base = specialSet.empty()
                                   ? string("!@#$%^&*_-+=")
                                   : specialSet;
                string usable;
                for (char c : base) {
                    // Omit potentially problematic characters if requested
                    if (omitAmbiguous && (c == '|' || c == '`')) continue;
                    usable += c;
                }
                if (usable.empty()) return '!';
                return usable[RandomNumber(0, (int)usable.size() - 1)];
            }
            default:
                return (char)RandomNumber('A', 'Z'); // Fallback to capital letter
        }
    }

    /**
     * @brief Generates a random sequence of characters.
     * @param Length The desired length of the string.
     */
    static string GenerateWord(enCharType CharType,
                               short Length,
                               const string &specialSet = "!@#$%^&*_-+=",
                               bool omitAmbiguous = false) {
        string word;
        word.reserve(Length);
        for (int i = 0; i < Length; ++i)
            word += GetRandomCharacter(CharType, specialSet, omitAmbiguous);
        return word;
    }

    /**
     * @brief Generates a highly secure, guaranteed complex password.
     * Guarantees that the password contains at least one of each requested character type.
     * 
     * @param Length The desired length of the password (minimum 12).
     * @param includeSpecial Whether to include special characters.
     * @param omitAmbiguous Whether to omit ambiguous characters like 0/O/1/l.
     * @return string The generated strong password.
     */
    static string GeneratePassword(short Length,
                                   bool includeSpecial = true,
                                   const string &specialSet = "!@#$%^&*_-+=",
                                   bool omitAmbiguous = false) {
        // Enforce a minimum length of 12 for strong passwords
        if (Length < 12) Length = 12;

        vector<char> pwd;
        pwd.reserve(Length);

        // Guarantee at least one of each base type is included
        pwd.push_back(GetRandomCharacter(SmallLetter,   specialSet, omitAmbiguous));
        pwd.push_back(GetRandomCharacter(CapitalLetter, specialSet, omitAmbiguous));
        pwd.push_back(GetRandomCharacter(Digit,         specialSet, omitAmbiguous));
        if (includeSpecial)
            pwd.push_back(GetRandomCharacter(SpecialCharacter, specialSet, omitAmbiguous));

        // Fill the rest of the password length with a mix of allowed characters
        enCharType mix = includeSpecial ? MixCharsAndSpecial : MixChars;
        while ((int)pwd.size() < Length)
            pwd.push_back(GetRandomCharacter(mix, specialSet, omitAmbiguous));

        // Shuffle the string to ensure the guaranteed characters aren't always at the beginning
        // Fisher-Yates shuffle implementation
        for (int i = (int)pwd.size() - 1; i > 0; --i) {
            int j = RandomNumber(0, i);
            swap(pwd[i], pwd[j]);
        }

        // Return final generated password string
        return string(pwd.begin(), pwd.end());
    }
};
