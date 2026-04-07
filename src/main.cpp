#include <iostream>
#include <fstream>

#include "parser.h"
#include "databaseManager.h"
#include "error.h"

int main(int argc, char* argv[]) {
    try {
        if(argc < 2) {
            throw RuntimeException(RuntimeError::FILE_NOT_PROVIDED, 0, 0, "", "");
        }
    }
    catch(const ArkException& e) {
        std::cerr << e.what();
        return 1;
    }

    std::ifstream file(argv[1]);
    try {
        if(!file) {
            throw RuntimeException(RuntimeError::FILE_NOT_FOUND, 0, 0, argv[1], "");
        } 
    }
    catch(const ArkException& e) {
        std::cerr << e.what();

        return 1;
    }

    DatabaseManager manager;

    std::string command;
    char ch;
    int currentLine = 1;
    int commandStartLine = 1;

    while(file.get(ch)) {
        if(ch == '\n') currentLine++;

        if(ch == '-' && file.peek() == '-') {
            while(file.get(ch) && ch != '\n');
            
            currentLine++;
            continue;
        }

        command += ch;

        if(ch == ';') {
            
            try {
                Parser parser(command, commandStartLine);
                parser.parse(manager); 
            }
            catch(const ArkException& e) {
                std::cerr << e.what() << "\n";

                return 1;
            }

            command.clear();

            commandStartLine = currentLine;
        }
    } 

    return 0;
}