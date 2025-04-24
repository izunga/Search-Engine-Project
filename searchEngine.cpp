
// searchEngine.cpp
#include "searchEngine.h" // Includes the header file that defines the SearchEngine class and its dependencies
#include "rapidjson/document.h" // Includes RapidJSON library for parsing JSON documents
#include "rapidjson/istreamwrapper.h" // Stream wrapper for integrating RapidJSON with file input streams
#include <filesystem> // Provides functions for filesystem operations (e.g., directory traversal)
#include <fstream> // For file input/output operations
#include <sstream> // For string stream processing
#include <algorithm> // For common algorithms like std::transform
#include <iostream> // For console input/output
#include <chrono> // For measuring time intervals

namespace fs = std::filesystem; // Creates an alias for the filesystem namespace

// Associates an organization with a file in the index
void SearchEngine::WordMap::associateOrg(const std::string& org, const std::string& filepath) {
    std::unordered_map<std::string, int> files; // A map to track files and their counts for the organization
    if (!orgIndex.find(org, files)) { // Check if the organization exists in the index
        files = std::unordered_map<std::string, int>(); // Initialize a new map if not found
    }
    files[filepath]++; // Increment the count for the file
    orgIndex.insert(org, files); // Update the index with the organization and its files
}

// Associates a person’s name with a file in the index
void SearchEngine::WordMap::associateName(const std::string& name, const std::string& filepath) {
    std::unordered_map<std::string, int> files;
    if (!nameIndex.find(name, files)) { // Check if the name exists in the index
        files = std::unordered_map<std::string, int>();
    }
    files[filepath]++;
    nameIndex.insert(name, files);
}

// Associates a word with a file in the index
void SearchEngine::WordMap::associateWord(const std::string& word, const std::string& filepath) {
    // Skip indexing empty words
    if (word.empty()) return;

    std::unordered_map<std::string, int> files;
    if (!wordIndex.find(word, files)) { // Check if the word exists in the index
        files = std::unordered_map<std::string, int>();
    }
    files[filepath]++;
    wordIndex.insert(word, files); // Update the index with the word and its files
}

// Loads saved indexes from file paths
bool SearchEngine::WordMap::load(const std::string& filenamepath, const std::string& osavePath,
                               const std::string& nsavePath, const std::string& wsavePath,
                               const std::string& fsavePath) {
    try {
        orgIndex.load(osavePath); // Load organization index
        nameIndex.load(nsavePath); // Load name index
        wordIndex.load(wsavePath); // Load word index
        return true; // Return true if loading succeeds
    } catch (const std::exception&) { // Catch exceptions if any errors occur during loading
        return false; // Return false if loading fails
    }
}

// Saves current indexes to specified file paths
void SearchEngine::WordMap::save(const std::string& filenamepath, const std::string& osavePath,
                               const std::string& nsavePath, const std::string& wsavePath,
                               const std::string& fsavePath) const {
    orgIndex.save(osavePath); // Save organization index
    nameIndex.save(nsavePath); // Save name index
    wordIndex.save(wsavePath); // Save word index
}

// Retrieves files associated with an organization
std::unordered_map<std::string, int> SearchEngine::WordMap::getFilesByOrg(const std::string& org) const {
    std::unordered_map<std::string, int> files;
    orgIndex.find(org, files); // Find files for the given organization
    return files; // Return the result
}

// Retrieves files associated with a name
std::unordered_map<std::string, int> SearchEngine::WordMap::getFilesByName(const std::string& name) const {
    std::unordered_map<std::string, int> files;
    nameIndex.find(name, files); // Find files for the given name
    return files;
}

// Retrieves files associated with a word
std::unordered_map<std::string, int> SearchEngine::WordMap::getFilesByWord(const std::string& word) const {
    std::unordered_map<std::string, int> files;
    wordIndex.find(word, files); // Find files for the given word
    return files;
}

// Alias for getFilesByWord, retrieves files for other contexts
std::unordered_map<std::string, int> SearchEngine::WordMap::getOtherFilesByWord(const std::string& word) const {
    return getFilesByWord(word);
}

// Constructor for the SearchEngine
SearchEngine::SearchEngine(const std::string& folderPath, const std::string& filenamepath,
                         const std::string& osavePath, const std::string& nsavePath,
                         const std::string& wsavePath, const std::string& fsavePath)
    : textProcessor() { // Initialize the text processor
    if (!wordMap.load(filenamepath, osavePath, nsavePath, wsavePath, fsavePath)) {
        buildFromScratch(folderPath); // Build the index if loading fails
        wordMap.save(filenamepath, osavePath, nsavePath, wsavePath, fsavePath); // Save the new index
    }
}

// Destructor
SearchEngine::~SearchEngine() {}

// Builds the index from scratch by processing JSON files
void SearchEngine::buildFromScratch(const std::string& folderPath) {
    std::cout << "Reading JSONs..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now(); // Start timing

    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) { // Iterate over files in folder
        if (entry.is_regular_file()) { // Process only regular files
            std::string filePath = entry.path().string(); // Get file path
            std::vector<std::unordered_set<std::string>> words = getRelevantData(filePath); // Extract relevant data

            // Process and index organizations
            for (const auto& word : words[0]) {
                std::string lowerWord = word;
                std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), ::tolower);
                wordMap.associateOrg(lowerWord, filePath);
            }

            // Process and index person names
            for (const auto& word : words[1]) {
                std::string lowerWord = word;
                std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), ::tolower);
                wordMap.associateName(lowerWord, filePath);
            }

            // Process and index words after applying text processing
            for (const auto& word : words[2]) {
                std::string processedWord = textProcessor.processWord(word);
                if (!processedWord.empty()) {
                    wordMap.associateWord(processedWord, filePath);
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now(); // End timing
    std::chrono::duration<double> duration = end - start;
    std::cout << "JSONs read in " << duration.count() << " seconds.\n"; // Output duration
}

std::unordered_set<std::string> SearchEngine::parse(const std::string& searchTerms) const {
    // Initialize an unordered set to store unique search terms.
    std::unordered_set<std::string> terms;

    // Create a stream from the input search terms string.
    std::istringstream iss(searchTerms);
    std::string term;

    // Process each term in the search input.
    while (iss >> term) {
        // Convert the term to lowercase for case-insensitive search.
        std::transform(term.begin(), term.end(), term.begin(), ::tolower);

        // Handle special search terms starting with "org:" or "person:".
        if (term.rfind("org:", 0) == 0 || term.rfind("person:", 0) == 0) {
            terms.insert(term); // Add the term as is.
        }
        // Handle negated terms (starting with '-').
        else if (term[0] == '-') {
            // Process the term (excluding the '-') using the text processor.
            std::string processedTerm = "-" + textProcessor.processWord(term.substr(1));
            if (processedTerm.length() > 1) { // Ensure there's something valid after the '-'.
                terms.insert(processedTerm); // Add the processed negated term.
            }
        }
        // Handle regular search terms.
        else {
            // Process the term using the text processor (e.g., stemming).
            std::string processedTerm = textProcessor.processWord(term);
            if (!processedTerm.empty()) { // Skip empty terms after processing.
                terms.insert(processedTerm); // Add the processed term to the set.
            }
        }
    }

    // Return the set of parsed search terms.
    return terms;
}
