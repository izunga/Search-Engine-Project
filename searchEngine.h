
#ifndef SEARCH_ENGINE_H  // Include guard to prevent multiple inclusions of the header file
#define SEARCH_ENGINE_H

#include "avl_tree.h"  // Include the AVLTree class for efficient data indexing
#include "text_processor.h"  // Include the TextProcessor class for text preprocessing and stemming
#include <string>  // Include string library for text handling
#include <vector>  // Include vector library for dynamic arrays
#include <unordered_set>  // Include unordered_set for fast lookups of unique elements
#include <unordered_map>  // Include unordered_map for key-value pair storage and quick access

class SearchEngine {  // Declaration of the SearchEngine class
private:
    TextProcessor textProcessor;  // Instance of TextProcessor to handle text preprocessing

    class WordMap {  // Nested WordMap class to manage associations between words and files

    private:
        AVLTree<std::unordered_map<std::string, int>> orgIndex;  // AVLTree to index organizations and their occurrences
        AVLTree<std::unordered_map<std::string, int>> nameIndex;  // AVLTree to index names and their occurrences
        AVLTree<std::unordered_map<std::string, int>> wordIndex;  // AVLTree to index words and their occurrences

    public:
        void associateOrg(const std::string& org, const std::string& filepath);  // Associate an organization with a file
        void associateName(const std::string& name, const std::string& filepath);  // Associate a name with a file
        void associateWord(const std::string& word, const std::string& filepath);  // Associate a word with a file

        bool load(const std::string& filenamepath, const std::string& osavePath,  // Load indices from file paths
                  const std::string& nsavePath, const std::string& wsavePath,
                  const std::string& fsavePath);

        void save(const std::string& filenamepath, const std::string& osavePath,  // Save indices to file paths
                  const std::string& nsavePath, const std::string& wsavePath,
                  const std::string& fsavePath) const;

        std::unordered_map<std::string, int> getFilesByOrg(const std::string& org) const;  // Retrieve files associated with an organization
        std::unordered_map<std::string, int> getFilesByName(const std::string& name) const;  // Retrieve files associated with a name
        std::unordered_map<std::string, int> getFilesByWord(const std::string& word) const;  // Retrieve files associated with a word
        std::unordered_map<std::string, int> getOtherFilesByWord(const std::string& word) const;  // Retrieve additional files associated with a word
    };

    WordMap wordMap;  // Instance of WordMap to manage word-to-file associations
    void buildFromScratch(const std::string& folderPath);  // Build indices from a folder of documents
    std::vector<std::unordered_set<std::string>> getRelevantData(const std::string& filePath) const;  // Extract relevant data from a file
    std::unordered_set<std::string> parse(const std::string& searchTerms) const;  // Parse search terms into individual words

public:
    SearchEngine(const std::string& folderPath,  // Constructor to initialize the search engine and indices
                 const std::string& filenamepath = "index.dat",
                 const std::string& osavePath = "org.dat",
                 const std::string& nsavePath = "name.dat",
                 const std::string& wsavePath = "word.dat",
                 const std::string& fsavePath = "freq.dat");
    ~SearchEngine();  // Destructor to clean up resources

    std::vector<std::string> search(const std::string& searchTerms) const;  // Perform a search and return matching file paths
};

#endif  // End of include guard
