#include <iostream>
#include <fstream>
#include <string>

int writeToFile() {
    std::ofstream outFile;
    std::string filename = "raw-audio.txt";

    // Open the file in append mode
    outFile.open(filename, std::ofstream::app);

    if (!outFile.is_open()) {
        std::cerr << "Failed to open " << filename << std::endl;
        return 1; // Return with error code
    }

    std::string line;
    std::cout << "Enter text to store in the file. Type 'exit' to stop.\n";

    // Read lines from the standard input
    while (std::getline(std::cin, line)) {
        // Check for the quit command
        if (line == "exit") {
            break;
        }

        // Write the line to the file
        outFile << line << std::endl;
    }

    outFile.close(); // Close the file
    return 0; // Successful completion
}