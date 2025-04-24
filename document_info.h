// document_info.h
#ifndef DOCUMENT_INFO_H  // Check if DOCUMENT_INFO_H is not defined
#define DOCUMENT_INFO_H  // Define DOCUMENT_INFO_H to prevent multiple inclusion

#include <string>  // Include the string library for string handling
#include <vector>  // Include the vector library for using vectors (not used in this file, but often used with collections)
#include <unordered_map>  // Include the unordered_map library for storing term frequencies in a hash map
#include <fstream>  // Include the fstream library for file input/output operations

// Define the DocumentInfo class
class DocumentInfo {
public:
    std::string title;  // The title of the document
    std::string publication;  // The publication where the document was published
    std::string date;  // The date of publication
    std::string filepath;  // The path to the document on disk
    std::unordered_map<std::string, int> termFrequencies;  // A map to store the frequency of terms in the document

    // Method to save the document's information to a file
    void save(std::ofstream& out) const {
        out << title << "\n"  // Write the title to the file
            << publication << "\n"  // Write the publication name to the file
            << date << "\n"  // Write the publication date to the file
            << filepath << "\n";  // Write the file path to the file
        
        out << termFrequencies.size() << "\n";  // Write the number of unique terms to the file
        for (const auto& [term, freq] : termFrequencies) {  // Loop through the term-frequency pairs
            out << term << " " << freq << "\n";  // Write each term and its frequency to the file
        }
    }

    // Method to load the document's information from a file
    void load(std::ifstream& in) {
        std::getline(in, title);  // Read the title from the file
        std::getline(in, publication);  // Read the publication name from the file
        std::getline(in, date);  // Read the publication date from the file
        std::getline(in, filepath);  // Read the file path from the file

        int size;  // Variable to hold the number of terms
        in >> size;  // Read the number of terms from the file
        in.ignore();  // Ignore the newline character after the size value

        for (int i = 0; i < size; i++) {  // Loop through the terms and their frequencies
            std::string term;  // Variable to hold the term
            int freq;  // Variable to hold the frequency of the term
            in >> term >> freq;  // Read the term and its frequency from the file
            termFrequencies[term] = freq;  // Store the term and its frequency in the map
        }
    }
};

#endif 
