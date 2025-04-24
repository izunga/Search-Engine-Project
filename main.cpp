// Include necessary header files
#include "searchEngine.h" // Provides the SearchEngine class for indexing and searching
#include "document_info.h" // Provides document-related utilities
#include "rapidjson/document.h" // For JSON parsing
#include "rapidjson/istreamwrapper.h" // Wrapper for JSON stream parsing
#include <iostream> // For input/output operations
#include <iomanip> // For output formatting
#include <string> // For string manipulation
#include <memory> // For smart pointers
#include <fstream> // For file handling
#include <limits> // For input validation
#include <filesystem> // For handling file system paths

// Alias the filesystem namespace for convenience
namespace fs = std::filesystem;

// Function to clear the console screen
void clearScreen() {
    #ifdef _WIN32
        system("cls"); // Clear screen for Windows
    #else
        system("clear"); // Clear screen for Unix-based systems
    #endif
}

// Function to display the main menu options
void displayMenu() {
    std::cout << "\nFinancial News Search Engine\n"; // Display title
    std::cout << "============================\n"; // Decorative separator
    std::cout << "1. Create new index\n"; // Option to create a new index
    std::cout << "2. Load existing index\n"; // Option to load an existing index
    std::cout << "3. Search\n"; // Option to perform a search
    std::cout << "4. Exit\n"; // Option to exit the program
    std::cout << "============================\n"; // Decorative separator
    std::cout << "Choice: "; // Prompt for user input
}

// Function to display search results
void displayResults(const std::vector<std::string>& results) {
    std::cout << "\nFound " << results.size() << " results:\n\n"; // Display the number of results

    int count = 0; // Initialize counter for results
    for (const auto& filepath : results) { // Iterate through the result file paths
        if (count++ >= 15) break; // Limit display to the first 15 results
        std::cout << count << ". File: " << filepath << "\n"; // Display the file path

        // Attempt to display the title from the JSON file
        std::ifstream file(filepath); // Open the file
        if (file) { 
            rapidjson::IStreamWrapper isw(file); // Wrap the file stream for JSON parsing
            rapidjson::Document document; // Create a JSON document
            if (!document.ParseStream(isw).HasParseError() && // Parse the file and check for errors
                document.HasMember("title") && // Check if the title field exists
                document["title"].IsString()) { // Ensure the title is a string
                std::cout << "   Title: " << document["title"].GetString() << "\n"; // Display the title
            }
        }
        std::cout << "\n"; // Add spacing between results
    }

    if (results.size() > 15) { // Notify if more results are available
        std::cout << "(Showing first 15 of " << results.size() << " results)\n";
    }
}

// Function to display the contents of a document
void displayDocument(const std::string& filepath) {
    fs::path fullPath = fs::absolute(filepath); // Convert relative path to absolute path

    // Print debug information about the file path
    std::cout << "Attempting to open: " << fullPath.string() << "\n";

    std::ifstream file(fullPath); // Open the file
    if (!file) { // Handle file open errors
        std::cout << "Error: Could not open file " << fullPath.string() << "\n";
        if (!fs::exists(fullPath)) { // Check if the file exists
            std::cout << "File does not exist at specified path.\n";
        } else {
            std::cout << "File exists but could not be opened. Check permissions.\n";
        }
        return; // Exit the function if file could not be opened
    }

    rapidjson::IStreamWrapper isw(file); // Wrap the file stream for JSON parsing
    rapidjson::Document document; // Create a JSON document
    document.ParseStream(isw); // Parse the JSON file

    if (document.HasParseError()) { // Handle JSON parsing errors
        std::cout << "Error: Failed to parse JSON file\n";
        return;
    }

    // Display document details
    std::cout << "\n===========================================\n\n";

    if (document.HasMember("title") && document["title"].IsString()) { // Display title if present
        std::cout << "Title: " << document["title"].GetString() << "\n\n";
    }

    if (document.HasMember("published") && document["published"].IsString()) { // Display published date if present
        std::cout << "Date: " << document["published"].GetString() << "\n\n";
    }

    if (document.HasMember("text") && document["text"].IsString()) { // Display text content if present
        std::cout << "Content:\n" << document["text"].GetString() << "\n";
    }

    if (document.HasMember("entities") && document["entities"].IsObject()) { // Display entities if present
        const auto& entities = document["entities"];

        if (entities.HasMember("organizations") && entities["organizations"].IsArray()) { // Display organizations
            std::cout << "\nOrganizations mentioned:\n";
            for (const auto& org : entities["organizations"].GetArray()) {
                if (org.HasMember("name") && org["name"].IsString()) {
                    std::cout << "- " << org["name"].GetString() << "\n";
                }
            }
        }

        if (entities.HasMember("persons") && entities["persons"].IsArray()) { // Display persons
            std::cout << "\nPersons mentioned:\n";
            for (const auto& person : entities["persons"].GetArray()) {
                if (person.HasMember("name") && person["name"].IsString()) {
                    std::cout << "- " << person["name"].GetString() << "\n";
                }
            }
        }
    }

    std::cout << "\n===========================================\n";

    std::cout << "\nPress Enter to continue..."; // Pause for user input
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Wait for Enter key
}

// Function to handle the search operation
void handleSearch(const std::unique_ptr<SearchEngine>& engine) {
    // Check if the engine is initialized
    if (!engine) {
        std::cout << "Please create or load an index first.\n";
        return;  // Return if engine is not initialized
    }

    // Prompt user to enter a search query
    std::cout << "Enter search query: ";
    std::string query;
    std::getline(std::cin, query);  // Get search query from user input

    // Check if the query is empty
    if (query.empty()) {
        std::cout << "Empty search query. Please try again.\n";
        return;  // Return if query is empty
    }

    // Perform search and store the results
    auto results = engine->search(query);
    displayResults(results);  // Display the search results

    // If there are results, allow the user to view articles
    if (!results.empty()) {
        while (true) {
            // Prompt user to select a result number or return to the menu
            std::cout << "\nEnter result number to view full article (0 to return to menu): ";
            std::string input;
            
            // Clear any leftover input in the input buffer
            std::cin.clear();
            std::getline(std::cin, input);  // Get input from user

            // If the input is empty, continue to next iteration
            if (input.empty()) {
                continue;
            }

            // Attempt to convert the input to an integer
            int resultNum;
            try {
                resultNum = std::stoi(input);  // Convert input to integer
            } catch (const std::exception&) {
                std::cout << "Invalid input. Please enter a number.\n";
                continue;  // If input is not a valid number, prompt again
            }

            // If the user enters 0, break out of the loop
            if (resultNum == 0) {
                break;
            }

            // If the result number is valid, display the article
            if (resultNum > 0 && resultNum <= std::min<int>(results.size(), 15)) {
                // Get the current working directory to display the document
                fs::path currentPath = fs::current_path();
                std::string fullPath = (currentPath / results[resultNum - 1]).string();
                displayDocument(fullPath);  // Display the selected document

                // Wait for user to press Enter before continuing
                std::cout << "\nPress Enter to continue...";
                std::cin.get(); // Wait for single Enter press

            } else {
                // Invalid result number, prompt the user to try again
                std::cout << "Invalid result number. Please try again.\n";
            }
        }
    }
}

// Main function for the program
int main(int argc, char* argv[]) {
    // Unique pointer to hold the search engine object
    std::unique_ptr<SearchEngine> engine;

    // Ensure the correct number of arguments are passed
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <command> [arguments]\n";
        std::cout << "Commands:\n";
        std::cout << "  index <directory>   - Create index from documents in directory\n";
        std::cout << "  query \"query text\" - Search the index\n";
        std::cout << "  ui                  - Start interactive interface\n";
        return 1;  // Return if incorrect number of arguments
    }

    // Store the command provided by the user
    std::string command = argv[1];

    // Case when the 'ui' command is used
    if (command == "ui") {
        std::string dataDir;
        std::cout << "Enter data directory path: ";
        std::getline(std::cin, dataDir);  // Get the directory path from user

        // Convert the directory path to an absolute path and check if it exists
        fs::path dataDirPath = fs::absolute(dataDir);
        if (!fs::exists(dataDirPath)) {
            std::cout << "Error: Directory " << dataDirPath.string() << " does not exist.\n";
            return 1;  // Return if the directory does not exist
        }

        // Store the current directory path for later use
        fs::path baseDir = fs::current_path();

        // Change to the specified data directory
        fs::current_path(dataDirPath);

        // Loop to provide the user with the interactive menu
        while (true) {
            displayMenu();  // Display the menu options

            std::string input;
            std::getline(std::cin, input);  // Get input from user

            // If the input is empty, continue to the next iteration
            if (input.empty()) {
                continue;
            }

            // Get the first character from the user's input as the choice
            char choice = input[0];

            // Switch statement to handle different menu options
            switch (choice) {
                case '1':  // Option to create a new index
                    std::cout << "Creating new index...\n";
                    try {
                        // Attempt to create a new search engine and index
                        engine = std::make_unique<SearchEngine>(".", "index.dat", "org.dat",
                                                              "name.dat", "word.dat", "freq.dat");
                        std::cout << "Index created successfully!\n";
                    } catch (const std::exception& e) {
                        std::cout << "Error creating index: " << e.what() << "\n";
                    }
                    break;

                case '2':  // Option to load an existing index
                    std::cout << "Loading existing index...\n";
                    try {
                        // Attempt to load an existing search engine and index
                        engine = std::make_unique<SearchEngine>(".", "index.dat", "org.dat",
                                                              "name.dat", "word.dat", "freq.dat");
                        std::cout << "Index loaded successfully!\n";
                    } catch (const std::exception& e) {
                        std::cout << "Error loading index: " << e.what() << "\n";
                    }
                    break;

                case '3':  // Option to perform a search
                    handleSearch(engine);  // Call the search handler function
                    break;

                case '4':  // Option to exit the program
                    // Change back to the original directory before exiting
                    fs::current_path(baseDir);
                    std::cout << "Goodbye!\n";
                    return 0;  // Exit the program

                default:  // If the input is invalid
                    std::cout << "Invalid choice. Please try again.\n";
                    break;
            }
        }
    }
    // Case when the 'index' command is used
    else if (command == "index") {
        // Ensure the directory argument is provided for indexing
        if (argc != 3) {
            std::cerr << "Missing directory argument for index command\n";
            return 1;  // Return if directory argument is missing
        }
        try {
            // Convert directory path to an absolute path and check if it exists
            fs::path indexPath = fs::absolute(argv[2]);
            if (!fs::exists(indexPath)) {
                std::cerr << "Error: Directory " << indexPath.string() << " does not exist.\n";
                return 1;  // Return if the directory does not exist
            }

            fs::current_path(indexPath);  // Change to the specified directory
            engine = std::make_unique<SearchEngine>(".", "index.dat", "org.dat",
                                                  "name.dat", "word.dat", "freq.dat");
            std::cout << "Index created successfully!\n";
        } catch (const std::exception& e) {
            std::cerr << "Error creating index: " << e.what() << "\n";
            return 1;  // Return if an error occurs during indexing
        }
    }
    // Case when the 'query' command is used
    else if (command == "query") {
        // Ensure the query argument is provided
        if (argc != 3) {
            std::cerr << "Missing query argument for query command\n";
            return 1;  // Return if the query argument is missing
        }
        try {
            // Create a new search engine and perform a search
            engine = std::make_unique<SearchEngine>(".", "index.dat", "org.dat",
                                                  "name.dat", "word.dat", "freq.dat");
            auto results = engine->search(argv[2]);  // Perform the search
            displayResults(results);  // Display the search results
        } catch (const std::exception& e) {
            std::cerr << "Error during search: " << e.what() << "\n";
            return 1;  // Return if an error occurs during search
        }
    }
    // Case when an unknown command is entered
    else {
        std::cerr << "Unknown command: " << command << "\n";
        return 1;  // Return if an unknown command is entered
    }

    return 0;  // Return 0 to indicate successful execution
}
