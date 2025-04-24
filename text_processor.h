
// text_processor.h
#ifndef TEXT_PROCESSOR_H
#define TEXT_PROCESSOR_H

// Include necessary standard libraries
#include <string>              // For string operations
#include <unordered_set>       // For efficient stopword lookups
#include <algorithm>           // For transformations like tolower
#include <cctype>              // For character checks like isalpha
#include <vector>              // For initializing the stopwords list

// Declaration of the TextProcessor class
class TextProcessor {
private:
    // Set to store stopwords for fast lookup
    std::unordered_set<std::string> stopwords;

    // Checks if a string ends with a specified suffix
    bool endsWith(const std::string& str, const std::string& suffix) const {
        if (str.length() < suffix.length()) return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }

    // Determines if a character at a specific position is a consonant
    bool isConsonant(const std::string& str, int i) const {
        char c = str[i];
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
            return false;
        if (c == 'y') {
            return (i == 0) ? true : !isConsonant(str, i - 1);
        }
        return true;
    }

    // Measures the number of vowel-consonant (VC) sequences in a word
    int measureConsecutiveVC(const std::string& str) const {
        int m = 0;
        bool prevC = true; // Tracks whether the previous character was a consonant
        for (int i = 0; i < str.length(); i++) {
            bool isC = isConsonant(str, i);
            if (prevC && !isC)
                m++; // Increment when switching from consonant to vowel
            prevC = isC;
        }
        return m;
    }

    // Checks if a word contains at least one vowel
    bool hasVowel(const std::string& str) const {
        for (int i = 0; i < str.length(); i++)
            if (!isConsonant(str, i))
                return true;
        return false;
    }

    // Checks if a word ends with a double consonant
    bool endsWithDoubleConsonant(const std::string& str) const {
        if (str.length() < 2) return false;
        return str[str.length()-1] == str[str.length()-2] &&
               isConsonant(str, str.length()-1);
    }

    // Checks if a word ends with a consonant-vowel-consonant pattern
    bool endsWithCVC(const std::string& str) const {
        if (str.length() < 3) return false;
        int j = str.length() - 1;
        return isConsonant(str, j) && !isConsonant(str, j-1) &&
               isConsonant(str, j-2) && str[j] != 'w' && str[j] != 'x' && str[j] != 'y';
    }

    // Replaces a suffix of a word with another string
    void replaceSuffix(std::string& str, const std::string& suffix, const std::string& replacement) const {
        if (str.length() >= suffix.length() &&
            str.substr(str.length() - suffix.length()) == suffix) {
            str.replace(str.length() - suffix.length(), suffix.length(), replacement);
        }
    }

    // First step of the Porter Stemmer algorithm: removes plural forms
    void step1a(std::string& str) const {
        if (endsWith(str, "sses"))
            replaceSuffix(str, "sses", "ss");
        else if (endsWith(str, "ies"))
            replaceSuffix(str, "ies", "i");
        else if (endsWith(str, "ss"))
            return;
        else if (endsWith(str, "s"))
            str.pop_back(); // Remove trailing 's'
    }

    // Second step of the Porter Stemmer algorithm: handles past tense and gerunds
    void step1b(std::string& str) const {
        if (endsWith(str, "eed")) {
            if (measureConsecutiveVC(str.substr(0, str.length()-3)) > 0)
                replaceSuffix(str, "eed", "ee");
        }
        else if ((endsWith(str, "ed") && hasVowel(str.substr(0, str.length()-2))) ||
                 (endsWith(str, "ing") && hasVowel(str.substr(0, str.length()-3)))) {
            if (endsWith(str, "ed"))
                replaceSuffix(str, "ed", "");
            else
                replaceSuffix(str, "ing", "");

            // Handle special cases after removing "ed" or "ing"
            if (endsWith(str, "at") || endsWith(str, "bl") || endsWith(str, "iz"))
                str += "e";
            else if (endsWithDoubleConsonant(str) &&
                     !endsWith(str, "l") && !endsWith(str, "s") && !endsWith(str, "z"))
                str.pop_back();
            else if (measureConsecutiveVC(str) == 1 && endsWithCVC(str))
                str += "e";
        }
    }

    // Third step of the Porter Stemmer algorithm: handles words ending in "y"
    void step1c(std::string& str) const {
        if (endsWith(str, "y") && hasVowel(str.substr(0, str.length()-1)))
            str[str.length()-1] = 'i'; // Replace "y" with "i"
    }

public:
    // Constructor initializes stopwords
    TextProcessor() {
        const std::vector<std::string> commonStopwords = {
            // Common stopwords for natural language processing
            "a", "about", "above", "after", "again", "against", "all", "am", "an", "and",
            // ... (continued for brevity)
        };
        stopwords.insert(commonStopwords.begin(), commonStopwords.end());
    }

    // Checks if a word is a stopword
    bool isStopword(const std::string& word) const {
        return stopwords.find(word) != stopwords.end();
    }

    // Stems a given word using the Porter Stemmer algorithm
    std::string stem(std::string word) const {
        if (word.length() <= 2) return word;

        // Convert the word to lowercase for consistent processing
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);

        // Apply Porter Stemming steps
        step1a(word);
        step1b(word);
        step1c(word);

        return word;
    }

    // Processes a word by removing stopwords and applying stemming
    std::string processWord(const std::string& word) const {
        if (isStopword(word)) return ""; // Skip stopwords
        return stem(word); // Return the stemmed word
    }
};

#endif // End of include guard for TEXT_PROCESSOR_H
